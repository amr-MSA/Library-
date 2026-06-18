package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"
	"strings"
)

// Define structures for Telegram API
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

type SendMessage struct {
	ChatID int64  `json:"chat_id"`
	Text   string `json:"text"`
}

// Global variable to store current model
var currentModel = "openai/gpt-4o-mini:free"

func main() {
	// 1. Trick Render: Start a fake web server on the required port to bypass port-scan timeout
	port := os.Getenv("PORT")
	if port == "" {
		port = "8080" // Default port if not set
	}
	
	go func() {
		http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
			fmt.Fprintf(w, "Bot is running perfectly!")
		})
		log.Printf("Fake web server listening on port %s", port)
		if err := http.ListenAndServe(":" + port, nil); err != nil {
			log.Printf("Web server error: %v", err)
		}
	}()

	// 2. Start the actual Telegram Bot logic
	botToken := os.Getenv("TELEGRAM_BOT_TOKEN")
	apiKey := os.Getenv("OPENROUTER_API_KEY")

	if botToken == "" || apiKey == "" {
		log.Fatal("Critical Error: TELEGRAM_BOT_TOKEN or OPENROUTER_API_KEY is missing!")
	}

	log.Println("Bot starting and long polling initiated...")
	offset := 0

	for {
		updates, err := getUpdates(botToken, offset)
		if err != nil {
			log.Printf("Error getting updates: %v", err)
			continue
		}

		for _, update := range updates {
			offset = update.UpdateID + 1
			if update.Message != nil && update.Message.Text != "" {
				handleMessage(botToken, apiKey, update.Message)
			}
		}
	}
}

func getUpdates(token string, offset int) ([]Update, error) {
	url := fmt.Sprintf("https://api.telegram.org/bot%s/getUpdates?offset=%d&timeout=30", token, offset)
	resp, err := http.Get(url)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	var res struct {
		Ok     bool     `json:"ok"`
		Result []Update `json:"result"`
	}
	if err := json.NewDecoder(resp.Body).Decode(&res); err != nil {
		return nil, err
	}
	return res.Result, nil
}

func handleMessage(botToken, apiKey string, msg *Message) {
	text := msg.Text
	chatID := msg.Chat.ID

	// Handle commands to switch models
	if strings.HasPrefix(text, "/") {
		switch text {
case "/start":
    msg := "Welcome! Available Free Models:\n" +
           "🤖 /model_gpt - GPT-4o Mini (Very Smart & Accurate)\n" +
           "🤖 /model_mistral - Mistral 7B (Fast & Reliable)"
    sendTelegramMessage(botToken, chatID, msg)


case "/model_gpt":
    currentModel = "openai/gpt-4o-mini:free"
    sendTelegramMessage(botToken, chatID, "⚡ Switched to GPT-4o Mini Free. Ask me anything!")

case "/model_mistral":
    currentModel = "mistralai/mistral-7b-instruct:free"
    sendTelegramMessage(botToken, chatID, "⚡ Switched to Mistral 7B Free. Ask me anything!")


}

		return
	}

	// Forward the prompt to OpenRouter
	reply, err := askOpenRouter(apiKey, text)
	if err != nil {
		sendTelegramMessage(botToken, chatID, "Error communication with AI provider.")
		log.Printf("OpenRouter error: %v", err)
		return
	}

	sendTelegramMessage(botToken, chatID, reply)
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

	// Strict struct parsing to eliminate interface conversion panics
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

	// Case 1: OpenRouter returned an explicit API error
	if res.Error.Message != "" {
		return "OpenRouter Error: " + res.Error.Message, nil
	}

	// Case 2: Success response with valid choices
	if len(res.Choices) > 0 {
		return res.Choices[0].Message.Content, nil
	}

	return "Could not process response. Empty choices array from API.", nil
}


	

func sendTelegramMessage(token string, chatID int64, text string) {
	url := fmt.Sprintf("https://api.telegram.org/bot%s/sendMessage", token)
	reqBody, _ := json.Marshal(SendMessage{ChatID: chatID, Text: text})
	http.Post(url, "application/json", bytes.NewBuffer(reqBody))
}
