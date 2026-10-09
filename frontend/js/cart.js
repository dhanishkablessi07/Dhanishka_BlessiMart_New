const CartPage = {
    init() {
        App.register('/cart', () => this.render());
    },

    async render() {
        if(!App.state.user) {
            App.navigate('#/login');
            return;
        }

        document.getElementById('app').innerHTML = `
            <div class="container" style="padding-top: 2rem;">
                <h2>Shopping Cart</h2>
                <div id="cart-content">
                    <div class="spinner"></div>
                </div>
            </div>
        `;
        await this.loadCart();
    },

    async loadCart() {
        try {
            const res = await App.api('GET', '/api/cart');
            if(res.ok) {
                const cart = await res.json();
                this.renderCart(cart);
            } else {
                document.getElementById('cart-content').innerHTML = `<p>Failed to load cart.</p>`;
            }
        } catch(e) {
            document.getElementById('cart-content').innerHTML = `<p>Error loading cart.</p>`;
        }
    },

    renderCart(cart) {
        const items = cart.items || [];
        const content = document.getElementById('cart-content');
        
        if(items.length === 0) {
            content.innerHTML = `
                <div class="empty-state">
                    <h3>Your cart is empty</h3>
                    <button class="btn btn-primary" style="margin-top:1rem;" onclick="App.navigate('#/products')">Continue Shopping</button>
                </div>
            `;
            return;
        }

        let total = 0;
        let html = `
            <div style="display: flex; gap: 2rem; flex-wrap: wrap;">
                <div style="flex: 2; min-width: 300px;">
                    <div class="card">
                        <table style="width: 100%;">
                            <thead>
                                <tr>
                                    <th>Product</th>
                                    <th>Price</th>
                                    <th>Quantity</th>
                                    <th>Subtotal</th>
                                    <th></th>
                                </tr>
                            </thead>
                            <tbody>
        `;

        items.forEach(item => {
            const subtotal = parseFloat(item.price) * item.quantity;
            total += subtotal;
            html += `
                <tr>
                    <td>
                        <div style="display:flex; align-items:center; gap:10px;">
                            <img src="${App.escapeHtml(item.image_url || 'https://via.placeholder.com/50')}" style="width:50px; height:50px; object-fit:cover; border-radius:4px;">
                            <a href="#/products/${item.product_id}">${App.escapeHtml(item.product_name)}</a>
                        </div>
                    </td>
                    <td>${App.formatPrice(item.price)}</td>
                    <td>
                        <input type="number" value="${item.quantity}" min="1" class="form-control" style="width:70px;" onchange="CartPage.updateQty(${item.id}, this.value)">
                    </td>
                    <td>${App.formatPrice(subtotal)}</td>
                    <td>
                        <button class="btn btn-sm btn-danger" onclick="CartPage.removeItem(${item.id})">&times;</button>
                    </td>
                </tr>
            `;
        });

        html += `
                            </tbody>
                        </table>
                    </div>
                </div>
                <div style="flex: 1; min-width: 300px;">
                    <div class="card" style="position: sticky; top: 80px;">
                        <h3>Order Summary</h3>
                        <div style="display: flex; justify-content: space-between; margin: 1rem 0;">
                            <span>Subtotal</span>
                            <strong>${App.formatPrice(total)}</strong>
                        </div>
                        <hr style="border:0; border-top:1px solid var(--border); margin-bottom:1rem;">
                        <button class="btn btn-primary btn-block" onclick="CartPage.showCheckoutModal(${total})">Proceed to Checkout</button>
                    </div>
                </div>
            </div>
            
            <div id="checkout-modal" class="modal-backdrop" style="display:none;">
                <div class="modal">
                    <div class="modal-header">
                        <h3>Checkout</h3>
                        <button class="close-modal" onclick="document.getElementById('checkout-modal').style.display='none'">&times;</button>
                    </div>
                    <div class="modal-body">
                        <form id="checkout-form">
                        <div class="form-group">
    <label>Shipping Name</label>
    <input type="text" name="shipping_name" class="form-control" required>
</div>

<div class="form-group">
    <label>Shipping Phone</label>
    <input type="tel" name="shipping_phone" class="form-control" required>
</div>

<div class="form-group">
    <label>Shipping Address</label>
    <textarea name="shipping_address" class="form-control" rows="3" required></textarea>
</div>
                            <div class="form-group">
                                <label>Payment Method</label>
                                <select name="payment_method" class="form-control">
                                    <option value="cod">Cash on Delivery</option>
                                    <option value="card">Credit Card (Dummy)</option>
                                </select>
                            </div>
                            <div style="margin-top:1.5rem; text-align:right;">
                                <h4>Total: ${App.formatPrice(total)}</h4>
                            </div>
                        </form>
                    </div>
                    <div class="modal-footer">
                        <button class="btn btn-secondary" onclick="document.getElementById('checkout-modal').style.display='none'">Cancel</button>
                        <button class="btn btn-primary" onclick="CartPage.placeOrder()">Place Order</button>
                    </div>
                </div>
            </div>
        `;
        content.innerHTML = html;
    },

    async updateQty(itemId, qty) {
        qty = parseInt(qty);
        if(qty < 1) return;
        try {
            const res = await App.api('PUT', `/api/cart/items/${itemId}`, { quantity: qty });
            if(res.ok) this.loadCart();
            else App.toast('Failed to update quantity', 'error');
        } catch(e) { App.toast('Network error', 'error'); }
    },

    async removeItem(itemId) {
        try {
            const res = await App.api('DELETE', `/api/cart/items/${itemId}`);
            if(res.ok) {
                App.toast('Item removed', 'success');
                App.updateCartBadge();
                this.loadCart();
            }
        } catch(e) { App.toast('Network error', 'error'); }
    },

    showCheckoutModal(total) {
        document.getElementById('checkout-modal').style.display = 'flex';
    },

    async placeOrder() {
        const form = document.getElementById('checkout-form');
        if(!form.checkValidity()) {
            form.reportValidity();
            return;
        }
        const data = Object.fromEntries(new FormData(form));
        try {
            const res = await App.api('POST', '/api/orders', data);
            if(res.ok) {
                const order = await res.json();
                App.toast('Order placed successfully!', 'success');
                App.updateCartBadge();
                App.navigate('#/orders');
            } else {
                const err = await res.json();
                App.toast(err.message || 'Failed to place order', 'error');
            }
        } catch(e) {
            App.toast('Network error', 'error');
        }
    }
};
