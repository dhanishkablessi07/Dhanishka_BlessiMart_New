const loginForm = document.getElementById("loginForm");
const message = document.getElementById("message");

loginForm.addEventListener("submit", async function (event) {
    event.preventDefault();

    const email = document.getElementById("email").value.trim();
    const password = document.getElementById("password").value;

    message.textContent = "Logging in...";

    try {
        const response = await fetch("/api/auth/login", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                email: email,
                password: password
            })
        });

        const data = await response.json();

        if (response.ok) {
            message.textContent = "Login successful!";
            window.location.href = "index.html";
        } else {
            message.textContent = data.message || "Invalid email or password.";
        }
    } catch (error) {
        console.error("Login error:", error);
        message.textContent = "Unable to connect to the server.";
    }
});