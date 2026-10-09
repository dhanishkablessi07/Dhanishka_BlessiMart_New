const AuthPage = {
    init() {
        App.register('/login', () => this.renderLogin());
        App.register('/register', () => this.renderRegister());
        App.register('/logout', () => this.logout());
    },

    renderLogin() {
        document.getElementById('app').innerHTML = `
            <div class="container" style="margin-top: 4rem;">
                <div class="card" style="max-width: 400px; margin: auto;">
                    <h2 class="text-center" style="text-align: center;">Login</h2>
                    <form id="login-form">
                        <div class="form-group">
                            <label>Email</label>
                            <input type="email" name="email" class="form-control" required>
                        </div>
                        <div class="form-group">
                            <label>Password</label>
                            <input type="password" name="password" class="form-control" required>
                        </div>
                        <button type="submit" class="btn btn-primary btn-block">Login</button>
                    </form>
                    <p style="text-align: center; margin-top: 1rem;">
                        Don't have an account? <a href="#/register">Register</a>
                    </p>
                </div>
            </div>
        `;

        document.getElementById('login-form').addEventListener('submit', async (e) => {
            e.preventDefault();
            const btn = e.target.querySelector('button');
            btn.disabled = true;
            btn.textContent = 'Logging in...';
            
            const data = Object.fromEntries(new FormData(e.target));
            try {
                const res = await App.api('POST', '/api/auth/login', data);
                if(res.ok) {
                    App.toast('Login successful', 'success');
                    await App.checkAuth();
                    App.updateNav();
                    App.updateCartBadge();
                    App.navigate('#/');
                } else {
                    const err = await res.json();
                    App.toast(err.message || err.error || 'Login failed', 'error');
                }
            } catch(e) {
                App.toast('Network error', 'error');
            } finally {
                btn.disabled = false;
                btn.textContent = 'Login';
            }
        });
    },

    renderRegister() {
        document.getElementById('app').innerHTML = `
            <div class="container" style="margin-top: 4rem;">
                <div class="card" style="max-width: 400px; margin: auto;">
                    <h2 class="text-center" style="text-align: center;">Register</h2>
                    <form id="register-form">
                        <div class="form-group">
    <label>Username</label>
    <input type="text" name="username" class="form-control" required>
</div>

<div class="form-group">
    <label>Full Name</label>
    <input type="text" name="full_name" class="form-control" required>
</div>
                        <div class="form-group">
                            <label>Email</label>
                            <input type="email" name="email" class="form-control" required>
                        </div>
                        <div class="form-group">
                            <label>Password</label>
                            <input type="password" name="password" class="form-control" required minlength="6">
                        </div>
                        <div class="form-group">
                            <label>Role</label>
                            <select name="role" class="form-control" required>
                                <option value="buyer">Buyer</option>
                                <option value="seller">Seller</option>
                            </select>
                        </div>
                        <button type="submit" class="btn btn-primary btn-block">Register</button>
                    </form>
                    <p style="text-align: center; margin-top: 1rem;">
                        Already have an account? <a href="#/login">Login</a>
                    </p>
                </div>
            </div>
        `;

        document.getElementById('register-form').addEventListener('submit', async (e) => {
            e.preventDefault();
            const btn = e.target.querySelector('button');
            btn.disabled = true;
            btn.textContent = 'Registering...';
            
            const data = Object.fromEntries(new FormData(e.target));
            try {
                const res = await App.api('POST', '/api/auth/register', data);
                if(res.ok) {
                    App.toast('Registration successful. Please login.', 'success');
                    App.navigate('#/login');
                } else {
                    const err = await res.json();
                    App.toast(err.message || err.error || 'Registration failed', 'error');
                }
            } catch(e) {
                App.toast('Network error', 'error');
            } finally {
                btn.disabled = false;
                btn.textContent = 'Register';
            }
        });
    },

    async logout() {
        try {
            await App.api('POST', '/api/auth/logout');
            App.state.user = null;
            App.state.cartCount = 0;
            App.updateNav();
            App.navigate('#/');
            App.toast('Logged out', 'success');
        } catch(e) {
            App.toast('Error logging out', 'error');
        }
    }
};
