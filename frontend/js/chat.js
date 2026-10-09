const ChatWidget = {
    initialized: false,

    init() {
        if (this.initialized) return;
        this.initialized = true;

        const widget = document.createElement("div");
        widget.id = "blessimart-chat";
        widget.innerHTML = `
            <style>
                #blessimart-chat {
                    position: fixed;
                    right: 20px;
                    bottom: 20px;
                    z-index: 9999;
                    font-family: Arial, sans-serif;
                }
                #bm-chat-toggle {
                    border: none;
                    border-radius: 28px;
                    padding: 14px 20px;
                    background: #2563eb;
                    color: white;
                    font-size: 15px;
                    cursor: pointer;
                    box-shadow: 0 3px 12px #0003;
                }
                #bm-chat-box {
                    display: none;
                    width: min(340px, calc(100vw - 40px));
                    height: 420px;
                    max-height: calc(100vh - 110px);
                    margin-bottom: 12px;
                    background: white;
                    border: 1px solid #ddd;
                    border-radius: 12px;
                    overflow: hidden;
                    box-shadow: 0 4px 18px #0003;
                }
                #bm-chat-header {
                    padding: 14px;
                    background: #2563eb;
                    color: white;
                    font-weight: bold;
                }
                #bm-chat-messages {
                    height: calc(100% - 125px);
                    padding: 12px;
                    overflow-y: auto;
                    background: #f8fafc;
                }
                .bm-chat-message {
                    margin: 8px 0;
                    padding: 10px 12px;
                    border-radius: 10px;
                    max-width: 88%;
                    white-space: pre-wrap;
                    overflow-wrap: anywhere;
                    font-size: 14px;
                    line-height: 1.4;
                }
                .bm-chat-bot {
                    background: #e5e7eb;
                    color: #111827;
                }
                .bm-chat-user {
                    background: #dbeafe;
                    color: #111827;
                    margin-left: auto;
                }
                #bm-chat-form {
                    display: flex;
                    gap: 6px;
                    padding: 10px;
                    border-top: 1px solid #ddd;
                }
                #bm-chat-input {
                    min-width: 0;
                    flex: 1;
                    padding: 10px;
                    border: 1px solid #ccc;
                    border-radius: 8px;
                    font-size: 14px;
                }
                #bm-chat-send {
                    border: none;
                    border-radius: 8px;
                    padding: 10px 12px;
                    background: #2563eb;
                    color: white;
                    cursor: pointer;
                }
                #bm-chat-send:disabled {
                    opacity: .6;
                    cursor: wait;
                }
            </style>

            <section id="bm-chat-box" aria-label="BlessiMart chat assistant">
                <div id="bm-chat-header">BlessiMart Assistant</div>
                <div id="bm-chat-messages" aria-live="polite">
                    <div class="bm-chat-message bm-chat-bot">
                        Hi! Ask me about products, cart, orders, reviews or seller features.
                    </div>
                </div>
                <form id="bm-chat-form">
                    <input id="bm-chat-input" maxlength="500"
                        placeholder="Type your question..." autocomplete="off"
                        aria-label="Your question" required>
                    <button id="bm-chat-send" type="submit">Send</button>
                </form>
            </section>

            <button id="bm-chat-toggle" type="button"
                aria-expanded="false">💬 Chat with us</button>
        `;

        document.body.appendChild(widget);

        const box = widget.querySelector("#bm-chat-box");
        const toggle = widget.querySelector("#bm-chat-toggle");
        const messages = widget.querySelector("#bm-chat-messages");
        const form = widget.querySelector("#bm-chat-form");
        const input = widget.querySelector("#bm-chat-input");
        const send = widget.querySelector("#bm-chat-send");

        toggle.addEventListener("click", () => {
            const opening = box.style.display !== "block";
            box.style.display = opening ? "block" : "none";
            toggle.setAttribute("aria-expanded", String(opening));

            if (opening) input.focus();
        });

        const addMessage = (text, isUser = false) => {
            const message = document.createElement("div");
            message.className = "bm-chat-message " +
                (isUser ? "bm-chat-user" : "bm-chat-bot");
            message.textContent = text;
            messages.appendChild(message);
            messages.scrollTop = messages.scrollHeight;
            return message;
        };

        form.addEventListener("submit", async (event) => {
            event.preventDefault();

            const message = input.value.trim();
            if (!message || send.disabled) return;

            addMessage(message, true);
            input.value = "";
            send.disabled = true;
            const waiting = addMessage("Thinking...");

            try {
                const response = await fetch("/api/chat", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify({ message })
                });

                const data = await response.json();
                waiting.remove();

                if (!response.ok) {
                    addMessage(data.message || "Sorry, something went wrong.");
                } else {
                    addMessage(data.message || "I couldn't find an answer.");
                }
            } catch (error) {
                waiting.remove();
                addMessage("Unable to connect. Please check that the server is running.");
            } finally {
                send.disabled = false;
                input.focus();
            }
        });
    }
};

document.addEventListener("DOMContentLoaded", () => {
    ChatWidget.init();
});