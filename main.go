package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"

	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api/v5"
)

// Global variables for configuration
var botToken = os.Getenv("TELEGRAM_BOT_TOKEN")
var openRouterKey = os.Getenv("OPENROUTER_API_KEY")
var currentModel = "google/gemini-2.5-flash" // Default ultra-fast free model

func main() {
	if botToken == "" || openRouterKey == "" {
		log.Fatal("Critical Error: Environment variables are missing.")
	}

	// Internal web server to satisfy Render's port binding requirement
	go func() {
		port := os.Getenv("PORT")
		if port == "" {
			port = "10000"
		}
		http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
			fmt.Fprintf(w, "Bot is running perfectly.")
		})
		log.Printf("Fake web server listening on port %s", port)
		if err := http.ListenAndServe(":"+port, nil); err != nil {
			log.Printf("Web server error: %v", err)
		}
	}()

	bot, err := tgbotapi.NewBotAPI(botToken)
	if err != nil {
		log.Fatalf("Telegram API connection error: %v", err)
	}

	log.Println("Bot starting and long polling initiated...")
	u := tgbotapi.NewUpdate(0)
	u.Timeout = 60

	updates := bot.GetUpdatesChan(u)

	for update := range updates {
		if update.Message == nil {
			continue
		}

		chatID := update.Message.Chat.ID
		text := update.Message.Text

		// Command parsing routing structure
		switch text {
		case "/start":
			msg := "Welcome! Available Strong Free Models:\n\n" +
				"🤖 /model_gemini - Gemini 2.5 Flash (Fast & Smart)\n" +
				"🤖 /model_gpt - GPT-4o Mini (Accurate & Creative)\n" +
				"🤖 /model_deepseek - DeepSeek V3 Free (Powerful & Logical)\n" +
				"🤖 /model_qwen - Qwen 2.5 72B Free (Excellent Arabic & Coding)\n\n" +
				"📝 Just send any text message to chat with the active model!"
			sendTelegramMessage(chatID, msg)

		case "/model_gemini":
			currentModel = "google/gemini-2.5-flash"
			sendTelegramMessage(chatID, "⚡ Switched to Gemini 2.5 Flash Free. Ask me anything!")

		case "/model_gpt":
			currentModel = "openai/gpt-4o-mini:free"
			sendTelegramMessage(chatID, "⚡ Switched to GPT-4o Mini Free. Ask me anything!")

		case "/model_deepseek":
			currentModel = "deepseek/deepseek-chat:free"
			sendTelegramMessage(chatID, "⚡ Switched to DeepSeek V3 Free. Ask me anything!")

		case "/model_qwen":
			currentModel = "qwen/qwen-2.5-72b-instruct:free"
			sendTelegramMessage(chatID, "⚡ Switched to Qwen 2.5 72B Free. Ask me anything!")

		default:
			// Route standard incoming message to the active OpenRouter endpoint
			reply, err := askOpenRouter(openRouterKey, text)
			if err != nil {
				sendTelegramMessage(chatID, "Error communicating with AI service.")
				log.Printf("OpenRouter API execution failure: %v", err)
				continue
			}
			sendTelegramMessage(chatID, reply)
		}
	}
}

func askOpenRouter(apiKey, prompt string) (string, error) {
	url := "https://openrouter.ai/api/v1/chat/completions"

	reqBody, _ := json.Marshal(map[string]interface{}{
		"model": currentModel,
		"messages": []map[string]string{
			{"role": "user", "content": prompt},
		},
	})

	req, _ := http.NewRequest("POST", url, bytes.NewBuffer(reqBody))
	req.Header.Set("Authorization", "Bearer "+apiKey)
	req.Header.Set("Content-Type", "application/json")

	client := &http.Client{}
	resp, err := client.Do(req)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	// Strict struct parsing to isolate structure failures from API responses
	var res struct {
		Choices []struct {
			Message struct {
				Content string `json:"content"`
			} `json:"message"`
		} `json:"choices"`
		Error struct {
			Message string `json:"message"`
		} `json:"error"`
	}

	if err := json.NewDecoder(resp.Body).Decode(&res); err != nil {
		return "", err
	}

	// Evaluate API errors safely
	if res.Error.Message != "" {
		return "OpenRouter Error: " + res.Error.Message, nil
	}

	if len(res.Choices) > 0 {
		return res.Choices[0].Message.Content, nil
	}

	return "Could not process response. Empty response array returned.", nil
}

func sendTelegramMessage(chatID int64, text string) {
	url := "https://api.telegram.org/bot" + botToken + "/sendMessage"
	reqBody, _ := json.Marshal(map[string]interface{}{
		"chat_id": chatID,
		"text":    text,
	})

	resp, err := http.Post(url, "application/json", bytes.NewBuffer(reqBody))
	if err != nil {
		log.Printf("Failed to transmit Telegram frame: %v", err)
		return
	}
	resp.Body.Close()
}
