const ProductDetailPage = {
    init() {
        App.register('/products', (params) => {
            if (params && params.length > 0 && params[0] !== '') {
                this.render(params[0]);
            } else {
                ProductsPage.render();
            }
        });
    },

    async render(id) {
        document.getElementById('app').innerHTML = `<div class="spinner"></div>`;

        try {
            const res = await App.api('GET', `/api/products/${id}`);

            if (res.ok) {
                const product = await res.json();
                this.renderProduct(product);
                this.loadReviews(id);
            } else {
                document.getElementById('app').innerHTML =
                    `<div class="container empty-state"><h3>Product not found</h3></div>`;
            }
        } catch (e) {
            document.getElementById('app').innerHTML =
                `<div class="container empty-state"><h3>Error loading product</h3></div>`;
        }
    },

    renderProduct(p) {
        const imgUrl = p.image_url ||
            'https://via.placeholder.com/600x400?text=No+Image';

        const inStock = parseInt(p.stock_quantity) > 0;

        let html = `
            <div class="container" style="padding-top: 2rem;">

                <div style="display: flex; flex-wrap: wrap; gap: 2rem; margin-bottom: 3rem;">

                    <div style="flex: 1; min-width: 300px;">
                        <img
                            src="${App.escapeHtml(imgUrl)}"
                            style="width: 100%; border-radius: 8px; box-shadow: var(--shadow);"
                            alt="${App.escapeHtml(p.name)}"
                        >
                    </div>

                    <div style="flex: 1; min-width: 300px;">

                        <h1 style="margin-bottom: 0.5rem;">
                            ${App.escapeHtml(p.name)}
                        </h1>

                        <p style="color: var(--text-light); margin-bottom: 1rem;">
                            By ${App.escapeHtml(p.seller_name || 'Seller')}
                            |
                            Category: ${App.escapeHtml(p.category_name || 'General')}
                        </p>

                        <div class="product-price"
                             style="font-size: 2rem; margin-bottom: 1rem;">
                            ${App.formatPrice(p.price)}
                        </div>

                        <p style="margin-bottom: 1rem;">
                            ${
                                inStock
                                ? `<span class="badge badge-success">
                                    In Stock (${p.stock_quantity})
                                   </span>`
                                : `<span class="badge badge-danger">
                                    Out of Stock
                                   </span>`
                            }
                        </p>

                        <p style="margin-bottom: 2rem; line-height: 1.8;">
                            ${App.escapeHtml(p.description || '')}
                        </p>

                        <div style="display: flex; gap: 1rem; align-items: center;">

                            <input
                                type="number"
                                id="qty"
                                value="1"
                                min="1"
                                max="${p.stock_quantity}"
                                class="form-control"
                                style="width: 80px;"
                                ${!inStock ? 'disabled' : ''}
                            >

                            <button
                                class="btn btn-primary"
                                onclick="ProductDetailPage.add(${p.id})"
                                ${!inStock ? 'disabled' : ''}
                                style="flex: 1; padding: 0.75rem;"
                            >
                                Add to Cart
                            </button>

                        </div>
                    </div>
                </div>

                <!-- REVIEWS SECTION -->

                <div class="card">

                    <h2>Reviews & Ratings</h2>

                    <div id="reviews-list">
                        <div class="spinner"></div>
                    </div>

                    ${
                        App.state.user &&
                        App.state.user.role === 'buyer'
                        ? `
                            <div style="
                                margin-top: 2rem;
                                padding-top: 1rem;
                                border-top: 1px solid var(--border);
                            ">

                                <h3>Write a Review</h3>

                                <form id="review-form">

                                    <div class="form-group">
                                        <label for="review-rating">
                                            Rating (1-5)
                                        </label>

                                        <select
                                            id="review-rating"
                                            class="form-control"
                                            style="width: 120px;"
                                        >
                                            <option value="5">
                                                ★★★★★ 5 Stars
                                            </option>

                                            <option value="4">
                                                ★★★★ 4 Stars
                                            </option>

                                            <option value="3">
                                                ★★★ 3 Stars
                                            </option>

                                            <option value="2">
                                                ★★ 2 Stars
                                            </option>

                                            <option value="1">
                                                ★ 1 Star
                                            </option>
                                        </select>
                                    </div>

                                    <div class="form-group">

                                        <label for="review-comment">
                                            Comment
                                        </label>

                                        <textarea
                                            id="review-comment"
                                            class="form-control"
                                            rows="3"
                                            placeholder="Write your review"
                                            required
                                        ></textarea>

                                    </div>

                                    <button
                                        type="button"
                                        class="btn btn-primary"
                                        onclick="ProductDetailPage.submitReview(${p.id})"
                                    >
                                        Submit Review
                                    </button>

                                </form>

                            </div>
                        `
                        : ''
                    }

                </div>

            </div>
        `;

        document.getElementById('app').innerHTML = html;
    },

    async loadReviews(id) {
        const list = document.getElementById('reviews-list');

        if (!list) return;

        try {
            const res = await App.api(
                'GET',
                `/api/products/${id}/reviews`
            );

            if (res.ok) {

                const reviews = await res.json();

                if (!reviews || reviews.length === 0) {
                    list.innerHTML = `<p>No reviews yet.</p>`;
                    return;
                }

                list.innerHTML = reviews.map(r => `
                    <div style="
                        padding: 1rem 0;
                        border-bottom: 1px solid var(--border);
                    ">

                        <div style="
                            display: flex;
                            justify-content: space-between;
                            margin-bottom: 0.5rem;
                        ">

                            <strong>
                                ${App.escapeHtml(r.buyer_name || 'User')}
                            </strong>

                            <span style="color: var(--text-light);">
                                ${new Date(r.created_at).toLocaleDateString()}
                            </span>

                        </div>

                        <div class="stars" style="margin-bottom: 0.5rem;">
                            ${ProductsPage.generateStars(Number(r.rating))}
                        </div>

                        <p>
                            ${App.escapeHtml(r.comment || '')}
                        </p>

                    </div>
                `).join('');

            } else {

                list.innerHTML = `<p>Failed to load reviews.</p>`;

            }

        } catch (e) {

            console.error('Failed to load reviews:', e);
            list.innerHTML = `<p>Error loading reviews.</p>`;

        }
    },

    async submitReview(productId) {

        const ratingElement = document.getElementById('review-rating');
        const commentElement = document.getElementById('review-comment');

        if (!ratingElement || !commentElement) return;

        const rating = parseInt(ratingElement.value);
        const comment = commentElement.value.trim();

        if (!comment) {
            App.toast('Please write a comment', 'warning');
            return;
        }

        if (rating < 1 || rating > 5) {
            App.toast('Please select a rating from 1 to 5', 'warning');
            return;
        }

        try {

            const res = await App.api(
                'POST',
                '/api/reviews',
                {
                    product_id: productId,
                    rating: rating,
                    comment: comment
                }
            );

            if (res.ok) {

                App.toast('Review submitted successfully', 'success');

                commentElement.value = '';

                await this.loadReviews(productId);

            } else {

                const err = await res.json();

                App.toast(
                    err.message || 'Failed to submit review',
                    'error'
                );
            }

        } catch (e) {

            console.error('Review submission error:', e);
            App.toast('Network error', 'error');

        }
    },

    async add(id) {

        if (!App.state.user) {
            App.navigate('#/login');
            return;
        }

        const qtyElement = document.getElementById('qty');

        if (!qtyElement) return;

        const qty = parseInt(qtyElement.value);

        if (!qty || qty < 1) {
            App.toast('Please enter a valid quantity', 'warning');
            return;
        }

        try {

            const res = await App.api(
                'POST',
                '/api/cart/items',
                {
                    product_id: id,
                    quantity: qty
                }
            );

            if (res.ok) {

                App.toast('Added to cart', 'success');
                App.updateCartBadge();

            } else {

                const err = await res.json();

                App.toast(
                    err.message || 'Failed to add to cart',
                    'error'
                );

            }

        } catch (e) {

            App.toast('Network error', 'error');

        }
    }
};

ProductDetailPage.init();