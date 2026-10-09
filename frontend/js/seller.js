const SellerPage = {
    init() {
        App.register('/seller', () => this.render());
    },

    async render() {
        const app = document.getElementById('app');

        app.innerHTML = `
            <div class="container" style="padding-top: 2rem;">

                <h2>Seller Dashboard</h2>

                <!-- Add Product -->
                <form id="productForm" style="margin-bottom: 2rem;">
                    <h3>Add Product</h3>

                    <input
                        id="productName"
                        type="text"
                        placeholder="Product name"
                        required
                    >

                    <br><br>

                    <textarea
                        id="productDescription"
                        placeholder="Description"
                    ></textarea>

                    <br><br>

                    <input
                        id="productPrice"
                        type="number"
                        step="0.01"
                        min="0"
                        placeholder="Price"
                        required
                    >

                    <br><br>

                    <input
                        id="productStock"
                        type="number"
                        min="0"
                        placeholder="Stock"
                        required
                    >

                    <br><br>

                    <input
                        id="productCategory"
                        type="number"
                        min="1"
                        placeholder="Category ID"
                        required
                    >

                    <br><br>

                    <input
                        id="productImage"
                        type="text"
                        placeholder="Image URL"
                    >

                    <br><br>

                    <button
                        type="submit"
                        class="btn btn-primary"
                    >
                        Add Product
                    </button>

                    <p id="sellerMessage"></p>
                </form>

                <hr>

                <!-- My Products -->
                <h3>My Products</h3>

                <div id="sellerProducts">
                    <div class="spinner"></div>
                </div>
                <hr>

<!-- Incoming Orders -->
<h3>Incoming Orders</h3>

<div id="sellerOrders">
    <div class="spinner"></div>
</div>

<hr>

            </div>
        `;

        document
            .getElementById('productForm')
            .addEventListener('submit', (event) => {
                this.addProduct(event);
            });

        await this.loadProducts();
await this.loadSellerOrders();
    },

    // =========================
    // ADD PRODUCT
    // =========================
    async addProduct(event) {
        event.preventDefault();

        const message =
            document.getElementById('sellerMessage');

        const product = {
            name: document
                .getElementById('productName')
                .value
                .trim(),

            description: document
                .getElementById('productDescription')
                .value
                .trim(),

            price: document
                .getElementById('productPrice')
                .value,

            stock: document
                .getElementById('productStock')
                .value,

            category_id: document
                .getElementById('productCategory')
                .value,

            image_url: document
                .getElementById('productImage')
                .value
                .trim()
        };

        message.textContent = 'Adding product...';

        try {
            const response = await App.api(
                'POST',
                '/api/products',
                product
            );

            if (response.ok) {
                message.textContent =
                    'Product added successfully!';

                document
                    .getElementById('productForm')
                    .reset();

                await this.loadProducts();

            } else {
                const error = await response.json();

                message.textContent =
                    error.error ||
                    error.message ||
                    'Failed to add product.';
            }

        } catch (error) {
            console.error(error);

            message.textContent =
                'Network error.';
        }
    },

    // =========================
    // LOAD SELLER PRODUCTS
    // =========================
    async loadProducts() {
        const container =
            document.getElementById('sellerProducts');

        if (!container) return;

        try {
            const response = await App.api(
                'GET',
                '/api/products'
            );

            if (!response.ok) {
                container.innerHTML =
                    '<p>Failed to load products.</p>';
                return;
            }

            const data = await response.json();

            const products = Array.isArray(data)
                ? data
                : (data.products || []);

            /*
             * Show only products belonging to
             * the currently logged-in seller.
             */
            const currentUser =
                App.state && App.state.user
                    ? App.state.user
                    : null;

            let sellerProducts = products;

            if (currentUser && currentUser.id) {
                sellerProducts = products.filter(product =>
                    String(product.seller_id) ===
                    String(currentUser.id)
                );
            }

            if (sellerProducts.length === 0) {
                container.innerHTML =
                    '<p>No products available.</p>';
                return;
            }

            container.innerHTML =
                sellerProducts.map(product => `

                    <div
                        class="product-card"
                        style="margin-bottom: 1rem;"
                    >

                        <div class="product-info">

                            <h3>
                                ${App.escapeHtml(
                                    product.name || ''
                                )}
                            </h3>

                            <p>
                                ${App.escapeHtml(
                                    product.description || ''
                                )}
                            </p>

                            <p>
                                Price:
                                ${App.formatPrice(
                                    product.price
                                )}
                            </p>

                            <p>
                                Stock:
                                ${product.stock}
                            </p>

                            <p>
                                Category ID:
                                ${product.category_id}
                            </p>

                            <button
                                class="btn btn-primary"
                                onclick="SellerPage.editProduct(${product.id})"
                            >
                                Edit
                            </button>

                            <button
                                class="btn btn-danger"
                                onclick="SellerPage.deleteProduct(${product.id})"
                            >
                                Delete
                            </button>

                        </div>

                    </div>

                `).join('');

        } catch (error) {
            console.error(error);

            container.innerHTML =
                '<p>Network error.</p>';
        }
    },
async loadSellerOrders() {
    const container = document.getElementById('sellerOrders');

    if (!container) return;

    try {
        const response = await App.api(
            'GET',
            '/api/seller/orders'
        );

        if (!response.ok) {
            container.innerHTML =
                '<p>Failed to load incoming orders.</p>';
            return;
        }

        const orders = await response.json();

        if (!orders || orders.length === 0) {
            container.innerHTML =
                '<p>No incoming orders yet.</p>';
            return;
        }

        container.innerHTML = orders.map(order => `
            <div class="card" style="margin-bottom: 1rem; padding: 1rem;">
                <h4>Order #${order.id}</h4>

                <p>
                    <strong>Order Number:</strong>
                    ${App.escapeHtml(order.order_number || '')}
                </p>

                <p>
                    <strong>Total:</strong>
                    ${App.formatPrice(order.total_amount)}
                </p>

                <p>
                    <strong>Status:</strong>
                    ${App.escapeHtml(order.order_status || '')}
                </p>

                <p>
                    <strong>Shipping Name:</strong>
                    ${App.escapeHtml(order.shipping_name || '')}
                </p>

                <p>
                    <strong>Phone:</strong>
                    ${App.escapeHtml(order.shipping_phone || '')}
                </p>

                <p>
                    <strong>Address:</strong>
                    ${App.escapeHtml(order.shipping_address || '')}
                </p>

                <button
                    class="btn btn-primary"
                    onclick="SellerPage.updateOrderStatus(${order.id}, 'Shipped')"
                >
                    Mark Shipped
                </button>

                <button
                    class="btn btn-success"
                    onclick="SellerPage.updateOrderStatus(${order.id}, 'Delivered')"
                >
                    Mark Delivered
                </button>
            </div>
        `).join('');

    } catch (error) {
        console.error(error);
        container.innerHTML =
            '<p>Network error while loading orders.</p>';
    }
},
async updateOrderStatus(orderId, status) {
    try {
        const response = await App.api(
            'PUT',
            `/api/orders/${orderId}/status`,
            { status: status }
        );

        if (response.ok) {
            App.toast(
                `Order status updated to ${status}`,
                'success'
            );

            await this.loadSellerOrders();
        } else {
            const error = await response.json();

            App.toast(
                error.error ||
                error.message ||
                'Failed to update order status',
                'error'
            );
        }

    } catch (error) {
        console.error(error);

        App.toast(
            'Network error',
            'error'
        );
    }
},
    // =========================
    // EDIT PRODUCT
    // =========================
    async editProduct(productId) {

        try {
            const response = await App.api(
                'GET',
                `/api/products/${productId}`
            );

            if (!response.ok) {
                App.toast(
                    'Failed to load product',
                    'error'
                );
                return;
            }

            const product =
                await response.json();

            const name = prompt(
                'Product name:',
                product.name || ''
            );

            if (name === null) return;

            const description = prompt(
                'Description:',
                product.description || ''
            );

            if (description === null) return;

            const price = prompt(
                'Price:',
                product.price || ''
            );

            if (price === null) return;

            const stock = prompt(
                'Stock:',
                product.stock || ''
            );

            if (stock === null) return;

            const category_id = prompt(
                'Category ID:',
                product.category_id || ''
            );

            if (category_id === null) return;

            const image_url = prompt(
                'Image URL:',
                product.image_url || ''
            );

            if (image_url === null) return;

            const updatedProduct = {
                name: name.trim(),
                description: description.trim(),
                price: price.trim(),
                stock: stock.trim(),
                category_id: category_id.trim(),
                image_url: image_url.trim()
            };

            const updateResponse =
                await App.api(
                    'PUT',
                    `/api/products/${productId}`,
                    updatedProduct
                );

            if (updateResponse.ok) {

                App.toast(
                    'Product updated successfully',
                    'success'
                );

                await this.loadProducts();

            } else {

                const error =
                    await updateResponse.json();

                App.toast(
                    error.error ||
                    error.message ||
                    'Failed to update product',
                    'error'
                );
            }

        } catch (error) {

            console.error(error);

            App.toast(
                'Network error',
                'error'
            );
        }
    },

    // =========================
    // DELETE PRODUCT
    // =========================
    async deleteProduct(productId) {

        const confirmed = confirm(
            'Are you sure you want to delete this product?'
        );

        if (!confirmed) {
            return;
        }

        try {

            const response = await App.api(
                'DELETE',
                `/api/products/${productId}`
            );

            if (response.ok) {

                App.toast(
                    'Product deleted successfully',
                    'success'
                );

                await this.loadProducts();

            } else {

                const error =
                    await response.json();

                App.toast(
                    error.error ||
                    error.message ||
                    'Failed to delete product',
                    'error'
                );
            }

        } catch (error) {

            console.error(error);

            App.toast(
                'Network error',
                'error'
            );
        }
    }
};

SellerPage.init();