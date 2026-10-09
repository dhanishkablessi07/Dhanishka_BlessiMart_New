const AdminPage = {
    init() {
        App.register('/admin', () => this.render());
    },

    async render() {
        if(!App.state.user || App.state.user.role !== 'admin') {
            App.navigate('#/');
            return;
        }

        document.getElementById('app').innerHTML = `
            <div class="container" style="padding-top: 2rem;">
                <h2>Admin Panel</h2>
                
                <div style="display: flex; gap: 1rem; margin-bottom: 2rem;">
                    <button class="btn btn-primary" onclick="AdminPage.showUsers()">Manage Users</button>
                    <button class="btn btn-secondary" onclick="AdminPage.showCategories()">Manage Categories</button>
                    <button class="btn btn-secondary" onclick="AdminPage.showOrders()">Manage Orders</button>
                    <button class="btn btn-danger" onclick="AdminPage.showProducts()">Manage Products</button>
                </div>

                <div id="admin-content">
                    <div class="spinner"></div>
                </div>
            </div>
            
            <!-- Category Modal -->
            <div id="category-modal" class="modal-backdrop" style="display:none;">
                <div class="modal">
                    <div class="modal-header">
                        <h3 id="cat-modal-title">Category</h3>
                        <button class="close-modal" onclick="document.getElementById('category-modal').style.display='none'">&times;</button>
                    </div>
                    <div class="modal-body">
                        <form id="category-form">
                            <input type="hidden" id="cat-id">
                            <div class="form-group">
                                <label>Name</label>
                                <input type="text" id="cat-name" class="form-control" required>
                            </div>
                            <div class="form-group">
                                <label>Description</label>
                                <textarea id="cat-desc" class="form-control" rows="3"></textarea>
                            </div>
                        </form>
                    </div>
                    <div class="modal-footer">
                        <button class="btn btn-secondary" onclick="document.getElementById('category-modal').style.display='none'">Cancel</button>
                        <button class="btn btn-primary" onclick="AdminPage.saveCategory()">Save</button>
                    </div>
                </div>
            </div>
        `;
        
        this.showUsers();
    },

    async showUsers() {
        const content = document.getElementById('admin-content');
        content.innerHTML = `<div class="spinner"></div>`;
        try {
            const res = await App.api('GET', '/api/admin/users');
            if(res.ok) {
                const users = await res.json();
                let html = `
                    <h3>Users</h3>
                    <div class="card table-responsive">
                        <table>
                            <thead>
                                <tr>
                                    <th>ID</th>
                                    <th>Username</th>
                                    <th>Email</th>
                                    <th>Role</th>
                                </tr>
                            </thead>
                            <tbody>
                `;
                users.forEach(u => {
                    html += `
                        <tr>
                            <td>${u.id}</td>
                            <td>${App.escapeHtml(u.username)}</td>
                            <td>${App.escapeHtml(u.email)}</td>
                            <td><span class="badge ${u.role === 'admin' ? 'badge-danger' : (u.role==='seller'?'badge-warning':'badge-primary')}">${u.role.toUpperCase()}</span></td>
                        </tr>
                    `;
                });
                html += `</tbody></table></div>`;
                content.innerHTML = html;
            }
        } catch(e) { content.innerHTML = `<p>Error loading users.</p>`; }
    },
async showOrders() {
    const content = document.getElementById('admin-content');
    content.innerHTML = `<div class="spinner"></div>`;

    try {
        const res = await App.api('GET', '/api/admin/orders');

        if (!res.ok) {
            content.innerHTML = `<p>Failed to load orders.</p>`;
            return;
        }

        const orders = await res.json();

        if (!orders || orders.length === 0) {
            content.innerHTML = `<p>No orders found.</p>`;
            return;
        }

        let html = `
            <h3>All Orders</h3>
            <div class="card table-responsive">
                <table>
                    <thead>
                        <tr>
                            <th>Order ID</th>
                            <th>Order Number</th>
                            <th>User ID</th>
                            <th>Total</th>
                            <th>Status</th>
                            <th>Payment</th>
                            <th>Created At</th>
                        </tr>
                    </thead>
                    <tbody>
        `;

        orders.forEach(order => {
            html += `
                <tr>
                    <td>#${order.id}</td>
                    <td>${App.escapeHtml(order.order_number || '')}</td>
                    <td>${App.escapeHtml(order.user_id || '')}</td>
                    <td>${App.formatPrice(order.total_amount)}</td>
                    <td>${App.escapeHtml(order.order_status || '')}</td>
                    <td>${App.escapeHtml(order.payment_status || '')}</td>
                    <td>${App.escapeHtml(order.created_at || '')}</td>
                </tr>
            `;
        });

        html += `
                    </tbody>
                </table>
            </div>
        `;

        content.innerHTML = html;

    } catch (e) {
        console.error(e);
        content.innerHTML = `<p>Error loading orders.</p>`;
    }
},
async showProducts() {
    const content = document.getElementById('admin-content');
    content.innerHTML = `<div class="spinner"></div>`;

    try {
        const res = await App.api('GET', '/api/products?limit=100');

        if (!res.ok) {
            content.innerHTML = `<p>Failed to load products.</p>`;
            return;
        }

        const data = await res.json();
        const products = data.products || data;

        if (!products || products.length === 0) {
            content.innerHTML = `<p>No products found.</p>`;
            return;
        }

        let html = `
            <h3>Manage Products</h3>
            <div class="card table-responsive">
                <table>
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>Product</th>
                            <th>Price</th>
                            <th>Stock</th>
                            <th>Action</th>
                        </tr>
                    </thead>
                    <tbody>
        `;

        products.forEach(product => {
            html += `
                <tr>
                    <td>#${product.id}</td>
                    <td>${App.escapeHtml(product.title || product.name || '')}</td>
                    <td>${App.formatPrice(product.price)}</td>
                    <td>${product.stock}</td>
                    <td>
                        <button class="btn btn-danger"
                            onclick="AdminPage.removeProduct(${product.id})">
                            Remove
                        </button>
                    </td>
                </tr>
            `;
        });

        html += `
                    </tbody>
                </table>
            </div>
        `;

        content.innerHTML = html;

    } catch (e) {
        console.error(e);
        content.innerHTML = `<p>Error loading products.</p>`;
    }
},
async removeProduct(id) {
    if (!confirm('Are you sure you want to remove this product?')) {
        return;
    }

    try {
        const res = await App.api('DELETE', `/api/products/${id}`);

        if (!res.ok) {
            alert('Failed to remove product.');
            return;
        }

        alert('Product removed successfully.');
        this.showProducts();

    } catch (e) {
        console.error(e);
        alert('Error removing product.');
    }
},
    async showCategories() {
        const content = document.getElementById('admin-content');
        content.innerHTML = `<div class="spinner"></div>`;
        try {
            const res = await App.api('GET', '/api/categories');
            if(res.ok) {
                const cats = await res.json();
                let html = `
                    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:1rem;">
                        <h3>Categories</h3>
                        <button class="btn btn-success" onclick="AdminPage.openCategoryModal()">+ Add Category</button>
                    </div>
                    <div class="card table-responsive">
                        <table>
                            <thead>
                                <tr>
                                    <th>ID</th>
                                    <th>Name</th>
                                    <th>Description</th>
                                    <th>Action</th>
                                </tr>
                            </thead>
                            <tbody>
                `;
                cats.forEach(c => {
                    html += `
                        <tr>
                            <td>${c.id}</td>
                            <td>${App.escapeHtml(c.name)}</td>
                            <td>${App.escapeHtml(c.description || '')}</td>
                            <td>
                                <button class="btn btn-sm btn-primary" onclick='AdminPage.openCategoryModal(${JSON.stringify(c).replace(/'/g, "&apos;")})'>Edit</button>
                            </td>
                        </tr>
                    `;
                });
                html += `</tbody></table></div>`;
                content.innerHTML = html;
            }
        } catch(e) { content.innerHTML = `<p>Error loading categories.</p>`; }
    },

    openCategoryModal(cat = null) {
        document.getElementById('cat-modal-title').textContent = cat ? 'Edit Category' : 'Add Category';
        document.getElementById('cat-id').value = cat ? cat.id : '';
        document.getElementById('cat-name').value = cat ? cat.name : '';
        document.getElementById('cat-desc').value = cat ? (cat.description || '') : '';
        document.getElementById('category-modal').style.display = 'flex';
    },

    async saveCategory() {
        const id = document.getElementById('cat-id').value;
        const name = document.getElementById('cat-name').value;
        const description = document.getElementById('cat-desc').value;
        
        if(!name) return App.toast('Name is required', 'warning');
        
        const data = { name, description };
        const method = id ? 'PUT' : 'POST';
        const url = id ? `/api/categories/${id}` : '/api/categories';
        
        try {
            const res = await App.api(method, url, data);
            if(res.ok) {
                App.toast('Category saved', 'success');
                document.getElementById('category-modal').style.display = 'none';
                this.showCategories();
            } else {
                App.toast('Failed to save category', 'error');
            }
        } catch(e) { App.toast('Network error', 'error'); }
    }
};
