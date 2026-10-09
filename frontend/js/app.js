// Global state
const App = {
  state: {
    user: null,
    cartCount: 0
  },

  routes: {},

  init() {
    this.checkAuth().then(() => {
      this.router();
      this.updateNav();
      if (this.state.user) this.updateCartBadge();
    });
    window.addEventListener('hashchange', () => this.router());
  },

  async checkAuth() {
    try {
      const res = await this.api('GET', '/api/auth/me');
      if (res.ok) { 
          const d = await res.json(); 
          this.state.user = d.user || d; 
      } else {
          this.state.user = null;
      }
    } catch {
      this.state.user = null;
    }
  },

  router() {
    const hash = window.location.hash || '#/';
    const pathParts = hash.replace('#','').split('?')[0].split('/').filter(Boolean);
    const route = pathParts.length > 0 ? pathParts[0] : '';
    const params = pathParts.slice(1);
    const key = '/' + route;
    
    // Clear app content before rendering
    document.getElementById('app').innerHTML = '<div class="spinner"></div>';
    
    if (this.routes[key]) {
      this.routes[key](params);
    
    } else {
      this.renderHome();
    }
  },

  register(path, fn) { 
      this.routes[path] = fn; 
  },

  navigate(hash) { 
      window.location.hash = hash; 
  },

  async api(method, url, body) {
    const opts = {
      method,
      headers: { 'Content-Type': 'application/json' },
      credentials: 'same-origin'
    };
    if (body) opts.body = JSON.stringify(body);
    return fetch(url, opts);
  },

  async updateCartBadge() {
    if (!this.state.user) return;
    try {
      const res = await this.api('GET', '/api/cart');
      if (res.ok) {
        const d = await res.json();
        this.state.cartCount = (d.items || []).length;
        const badge = document.getElementById('cart-badge');
        if (badge) {
          badge.textContent = this.state.cartCount || '';
          badge.style.display = this.state.cartCount > 0 ? 'inline' : 'none';
        }
      }
    } catch {}
  },

  updateNav() {
    const navUser = document.getElementById('nav-user');
    const navAuth = document.getElementById('nav-auth');
    if (!navUser || !navAuth) return;
    
    if (this.state.user) {
      navUser.style.display = 'flex';
      navAuth.style.display = 'none';
      const nameEl = document.getElementById('nav-username');
      if (nameEl) nameEl.textContent = this.state.user.username || this.state.user.email;
      
      const sl = document.getElementById('nav-seller');
      if (sl) sl.style.display = this.state.user.role === 'seller' ? 'inline' : 'none';
      
      const al = document.getElementById('nav-admin');
      if (al) al.style.display = this.state.user.role === 'admin' ? 'inline' : 'none';
    } else {
      navUser.style.display = 'none';
      navAuth.style.display = 'flex';
      
      const sl = document.getElementById('nav-seller');
      if (sl) sl.style.display = 'none';
      const al = document.getElementById('nav-admin');
      if (al) al.style.display = 'none';
    }
  },

  toast(msg, type = 'info') {
    const c = document.getElementById('toast-container');
    if (!c) return;
    const t = document.createElement('div');
    t.className = `toast toast-${type}`;
    t.textContent = msg;
    c.appendChild(t);
    setTimeout(() => t.remove(), 3500);
  },

  renderHome() {
    document.getElementById('app').innerHTML = `
      <section class="hero">
        <div class="container">
          <h1>Welcome to Dhanishka Blessi Mart</h1>
          <p>Discover amazing products from trusted sellers</p>
          <button class="btn btn-primary" style="font-size: 1.2rem; padding: 0.75rem 2rem;" onclick="App.navigate('#/products')">Shop Now</button>
        </div>
      </section>
      <div class="container" style="margin-top: 2rem;">
        <h2>Featured Products</h2>
        <div id="home-featured-products" class="product-grid">
           <div class="spinner"></div>
        </div>
      </div>
    `;
    if(window.ProductsPage) ProductsPage.renderFeatured('home-featured-products');
  },

  formatPrice(p) { return '\u20B9' + parseFloat(p).toFixed(2); },
  
  escapeHtml(s) {
    return String(s || '').replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
  }
};
