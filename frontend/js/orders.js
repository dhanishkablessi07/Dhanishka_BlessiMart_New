const OrdersPage = {
    init() {
        App.register('/orders', (params) => {
            if(params && params.length > 0 && params[0] !== '') {
                this.renderDetail(params[0]);
            } else {
                this.renderList();
            }
        });
    },

    async renderList() {
        if(!App.state.user) return App.navigate('#/login');

        document.getElementById('app').innerHTML = `
            <div class="container" style="padding-top: 2rem;">
                <h2>My Orders</h2>
                <div id="orders-content">
                    <div class="spinner"></div>
                </div>
            </div>
        `;

        try {
            const res = await App.api('GET', '/api/orders');
            if(res.ok) {
                const orders = await res.json();
                const content = document.getElementById('orders-content');
                if(orders.length === 0) {
                    content.innerHTML = `<div class="empty-state"><p>You haven't placed any orders yet.</p></div>`;
                    return;
                }
                let html = `
                    <div class="card table-responsive">
                        <table>
                            <thead>
                                <tr>
                                    <th>Order ID</th>
                                    <th>Date</th>
                                    <th>Total</th>
                                    <th>Status</th>
                                    <th>Action</th>
                                </tr>
                            </thead>
                            <tbody>
                `;
                orders.forEach(o => {
                    let badgeClass = 'badge-primary';
                    if(o.order_status === 'delivered') badgeClass = 'badge-success';
                    if(o.order_status === 'cancelled') badgeClass = 'badge-danger';
                    if(o.order_status === 'shipped') badgeClass = 'badge-warning';

                    html += `
                        <tr>
                            <td>#${o.id}</td>
                            <td>${new Date(o.created_at).toLocaleDateString()}</td>
                            <td>${App.formatPrice(o.total_amount)}</td>
                            <td><span class="badge ${badgeClass}">${o.order_status.toUpperCase()}</span></td>
                            <td><button class="btn btn-sm btn-outline" style="color:var(--primary); border-color:var(--primary)" onclick="App.navigate('#/orders/${o.id}')">View Details</button></td>
                        </tr>
                    `;
                });
                html += `</tbody></table></div>`;
                content.innerHTML = html;
            }
        } catch(e) {
            document.getElementById('orders-content').innerHTML = `<p>Error loading orders.</p>`;
        }
    },

    async renderDetail(id) {
        document.getElementById('app').innerHTML = `<div class="spinner"></div>`;
        try {
            const res = await App.api('GET', `/api/orders/${id}`);
            if(res.ok) {
                const order = await res.json();
                this.showDetail(order);
            } else {
                document.getElementById('app').innerHTML = `<div class="container empty-state"><h3>Order not found</h3></div>`;
            }
        } catch(e) {
            document.getElementById('app').innerHTML = `<div class="container empty-state"><h3>Error loading order</h3></div>`;
        }
    },

    showDetail(order) {
        let badgeClass = 'badge-primary';
        if(order.order_status === 'delivered') badgeClass = 'badge-success';
        if(order.order_status === 'cancelled') badgeClass = 'badge-danger';

        let html = `
            <div class="container" style="padding-top: 2rem;">
                <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:1rem;">
                    <h2>Order #${order.id}</h2>
                    <span class="badge ${badgeClass}" style="font-size:1rem; padding:0.5rem 1rem;">${order.order_status.toUpperCase()}</span>
                </div>
                
                <div class="card" style="margin-bottom:2rem;">
                    <p><strong>Date:</strong> ${new Date(order.created_at).toLocaleString()}</p>
                    <p><strong>Shipping Address:</strong> ${App.escapeHtml(order.shipping_address)}</p>
                    <p><strong>Total Amount:</strong> ${App.formatPrice(order.total_amount)}</p>
                </div>

                <h3>Items</h3>
                <div class="card table-responsive">
                    <table>
                        <thead>
                            <tr>
                                <th>Product</th>
                                <th>Price</th>
                                <th>Qty</th>
                                <th>Subtotal</th>
                            </tr>
                        </thead>
                        <tbody>
        `;

        const items = order.items || [];
        let reviewHtml = '';

if (order.order_status === 'Delivered') {
    reviewHtml = `
        <div class="card" style="margin-top:1.5rem;">
            <h3>Review Product</h3>
            <form onsubmit="OrdersPage.submitReview(event)">
                <input type="hidden" name="product_id" value="${items[0].product_id}">
                
                <label>Rating</label>
                <select name="rating" required>
                    <option value="">Select rating</option>
                    <option value="5">★★★★★ 5</option>
                    <option value="4">★★★★ 4</option>
                    <option value="3">★★★ 3</option>
                    <option value="2">★★ 2</option>
                    <option value="1">★ 1</option>
                </select>

                <label>Comment</label>
                <textarea name="comment" rows="3"
                    placeholder="Write your review"></textarea>

                <button type="submit" class="btn btn-primary">
                    Submit Review
                </button>
            </form>
        </div>
    `;
}
        items.forEach(item => {
            html += `
                <tr>
                    <td><a href="#/products/${item.product_id}">${App.escapeHtml(item.name)}</a></td>
                    <td>${App.formatPrice(item.unit_price)}</td>
                    <td>${item.quantity}</td>
                    <td>${App.formatPrice(parseFloat(item.unit_price) * item.quantity)}</td>
                </tr>
            `;
        });

        html += `
                        </tbody>
                    </table>
                </div>
                <button class="btn btn-secondary" style="margin-top:1rem;" onclick="App.navigate('#/orders')">&larr; Back to Orders</button>
            </div>
        `;
        html += reviewHtml;
        document.getElementById('app').innerHTML = html;
    },
    async submitReview(event) {
    event.preventDefault();

    const form = event.target;
    const data = Object.fromEntries(new FormData(form));

    try {
        const res = await App.api('POST', '/api/reviews', {
            product_id: data.product_id,
            rating: Number(data.rating),
            comment: data.comment
        });

        if (res.ok) {
            alert('Review submitted successfully!');
        } else {
            const error = await res.json();
            alert(error.message || 'Failed to submit review.');
        }
    } catch (e) {
        alert('Network error.');
    }
},
};
