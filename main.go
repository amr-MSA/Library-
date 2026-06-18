package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"strings"
)

type Update struct {
	UpdateID int     `json:"update_id"`
	Message  *Message `json:"message"`
}

type Message struct {
	Chat struct {
		ID int64 `json:"id"`
	} `json:"chat"`
	Text string `json:"text"`
}

var userModels = make(map[int64]string)

func main() {
	botToken := os.Getenv("TELEGRAM_BOT_TOKEN")
	if botToken == "" {
		log.Fatal("TELEGRAM_BOT_TOKEN is required")
	}

	log.Println("Bot starting...")
	offset := 0

	for {
		url := fmt.Sprintf("https://api.telegram.org/bot%s/getUpdates?offset=%d&timeout=30", botToken, offset)
		resp, err := http.Get(url)
		if err != nil {
			continue
		}

		var result struct {
			Ok     bool     `json:"ok"`
			Result []Update `json:"result"`
		}

		if err := json.NewDecoder(resp.Body).Decode(&result); err != nil {
			resp.Body.Close()
			continue
		}
		resp.Body.Close()

		for _, update := range result.Result {
			offset = update.UpdateID + 1
			if update.Message != nil && update.Message.Text != "" {
				go handleMessage(botToken, update.Message)
			}
		}
	}
}

func handleMessage(botToken string, msg *Message) {
	chatID := msg.Chat.ID
	text := msg.Text

	if strings.HasPrefix(text, "/start") {
		sendTelegram(botToken, chatID, "أهلاً بك! أنا بوت الذكاء الاصطناعي الموحد.\n\nاختر النموذج الذي تريد التحدث معه بإرسال الأمر المناسب:\n/model gemini\n/model deepseek\n/model claude\n/model kimi")
		return
	}

	if strings.HasPrefix(text, "/model ") {
		modelChoice := strings.TrimSpace(strings.TrimPrefix(text, "/model "))
		var fullModelName string

		switch modelChoice {
		case "gemini":
			fullModelName = "google/gemini-2.5-flash"
		case "deepseek":
			fullModelName = "deepseek/deepseek-chat"
		case "claude":
			fullModelName = "anthropic/claude-3.5-sonnet"
		case "kimi":
			fullModelName = "moonshotai/moonshot-v1"
		default:
			sendTelegram(botToken, chatID, "نموذج غير معروف. الخيارات المتاحة: gemini, deepseek, claude, kimi")
			return
		}

		userModels[chatID] = fullModelName
		sendTelegram(botToken, chatID, fmt.Sprintf("تم التحويل بنجاح إلى النموذج: %s", modelChoice))
		return
	}

	currentModel := userModels[chatID]
	if currentModel == "" {
		currentModel = "google/gemini-2.5-flash" // الافتراضي
	}

	sendTelegram(botToken, chatID, "جاري التفكير برأس "+currentModel+"... ⏳")
	aiResponse := askOpenRouter(currentModel, text)
	sendTelegram(botToken, chatID, aiResponse)
}

func askOpenRouter(model, prompt string) string {
	apiKey := os.Getenv("OPENROUTER_API_KEY")
	if apiKey == "" {
		return "خطأ: لم يتم ضبط مفتاح OpenRouter بنجاح في السيرفر."
	}

	url := "https://openrouter.ai/api/v1/chat/completions"
	
	payload := map[string]interface{}{
		"model": model,
		"messages": []map[string]string{
			{"role": "user", "content": prompt},
		},
	}

	jsonPayload, _ := json.Marshal(payload)
	req, _ := http.NewRequest("POST", url, bytes.NewBuffer(jsonPayload))
	req.Header.Set("Authorization", "Bearer "+apiKey)
	req.Header.Set("Content-Type", "application/json")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return "فشل الاتصال بـ OpenRouter."
	}
	defer resp.Body.Close()

	body, _ := io.ReadAll(resp.Body)
	
	var apiResp struct {
		Choices []struct {
			Message struct {
				Content string `json:"content"`
			} `json:"message"`
		} `json:"choices"`
	}

	if err := json.Unmarshal(body, &apiResp); err != nil || len(apiResp.Choices) == 0 {
		return "حدث خطأ أثناء معالجة رد الذكاء الاصطناعي."
	}

	return apiResp.Choices[0].Message.Content
}

func sendTelegram(token string, chatID int64, text string) {
	url := fmt.Sprintf("https://api.telegram.org/bot%s/sendMessage", token)
	payload := map[string]interface{}{
		"chat_id": chatID,
		"text":    text,
	}
	jsonPayload, _ := json.Marshal(payload)
	http.Post(url, "application/json", bytes.NewBuffer(jsonPayload))
}
