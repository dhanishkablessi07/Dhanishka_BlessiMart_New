const ProductsPage = {
    init() {
        App.register('/products', () => this.render());
    },

    async render() {
        const urlParams = new URLSearchParams(window.location.hash.split('?')[1]);
        const q = urlParams.get('q') || '';
        const cat = urlParams.get('category') || '';
        
        document.getElementById('app').innerHTML = `
            <div class="container" style="padding-top: 2rem;">
                <div class="search-bar">
                    <input type="text" id="search-input" class="form-control" placeholder="Search products..." value="${App.escapeHtml(q)}">
                    <select id="category-select" class="form-control" style="width: auto;">
                        <option value="">All Categories</option>
                    </select>
                    <button id="search-btn" class="btn btn-primary">Search</button>
                </div>
                <div id="products-grid" class="product-grid">
                    <div class="spinner"></div>
                </div>
            </div>
        `;

        document.getElementById('search-btn').addEventListener('click', () => {
            const sq = document.getElementById('search-input').value;
            const sc = document.getElementById('category-select').value;
            let hash = '#/products?';
            if(sq) hash += `q=${encodeURIComponent(sq)}&`;
            if(sc) hash += `category=${encodeURIComponent(sc)}`;
            App.navigate(hash);
        });

        await this.loadCategories(cat);
        await this.loadProducts(q, cat);
    },

    async loadCategories(selected) {
        try {
            const res = await App.api('GET', '/api/categories');
            if (res.ok) {
                const cats = await res.json();
                const sel = document.getElementById('category-select');
                if(!sel) return;
                cats.forEach(c => {
                    const opt = document.createElement('option');
                    opt.value = c.id;
                    opt.textContent = c.name;
                    if(c.id.toString() === selected) opt.selected = true;
                    sel.appendChild(opt);
                });
            }
        } catch(e) { console.error('Failed to load categories'); }
    },

    async loadProducts(q, cat) {
        const grid = document.getElementById('products-grid');
        if(!grid) return;
        
        try {
            let url = '/api/products?';
            if(q) url += `search=${encodeURIComponent(q)}&`;
            if(cat) url += `category=${encodeURIComponent(cat)}`;
            
            const res = await App.api('GET', url);
            if(res.ok) {
                const data = await res.json();
                const products = Array.isArray(data) ? data : (data.products || []);
                
                if(products.length === 0) {
                    grid.innerHTML = `<div class="empty-state" style="grid-column: 1/-1;"><h3>No products found</h3></div>`;
                    return;
                }
                
                grid.innerHTML = products.map(p => this.createProductCard(p)).join('');
            } else {
                grid.innerHTML = `<div class="empty-state" style="grid-column: 1/-1;"><h3>Failed to load products</h3></div>`;
            }
        } catch(e) {
            grid.innerHTML = `<div class="empty-state" style="grid-column: 1/-1;"><h3>Network error</h3></div>`;
        }
    },

    async renderFeatured(containerId) {
        const grid = document.getElementById(containerId);
        if(!grid) return;
        try {
            const res = await App.api('GET', '/api/products?limit=8');
            if(res.ok) {
                const data = await res.json();
                const products = Array.isArray(data) ? data : (data.products || []);
                grid.innerHTML = products.map(p => this.createProductCard(p)).join('');
            }
        } catch(e) {
            grid.innerHTML = `<p>Could not load featured products.</p>`;
        }
    },

    createProductCard(p) {
        const rating = parseFloat(p.average_rating) || 0;
        const imgUrl = p.image_url || 'https://via.placeholder.com/300x200?text=No+Image';
        return `
            <div class="product-card">
                <div class="product-img-wrapper">
                    <img src="${App.escapeHtml(imgUrl)}" class="product-img" alt="${App.escapeHtml(p.name)}">
                </div>
                <div class="product-info">
                    <h3 class="product-title"><a href="#/products/${p.id}">${App.escapeHtml(p.name)}</a></h3>
                    <div class="product-price">${App.formatPrice(p.price)}</div>
                    <div class="product-seller">Sold by: ${App.escapeHtml(p.seller_name || 'Seller')}</div>
                    <div class="stars" style="margin-bottom: 1rem;">
                        ${this.generateStars(rating)} <span style="color:#666; font-size:0.9em">(${rating.toFixed(1)})</span>
                    </div>
                    <button class="btn btn-primary btn-block" onclick="ProductsPage.addToCart(${p.id})">Add to Cart</button>
                </div>
            </div>
        `;
    },

    generateStars(rating) {
        let stars = '';
        for(let i=1; i<=5; i++) {
            if(rating >= i) stars += '★';
            else stars += '<span class="stars-empty">★</span>';
        }
        return stars;
    },

    async addToCart(productId) {
        if(!App.state.user) {
            App.toast('Please login to add to cart', 'warning');
            App.navigate('#/login');
            return;
        }
        try {
            const res = await App.api('POST', '/api/cart/items', { product_id: productId, quantity: 1 });
            if(res.ok) {
                App.toast('Added to cart', 'success');
                App.updateCartBadge();
            } else {
                const err = await res.json();
                App.toast(err.message || 'Failed to add to cart', 'error');
            }
        } catch(e) {
            App.toast('Network error', 'error');
        }
    }
};
ProductsPage.init();