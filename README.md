# Dhanishka_Blessi_Mart

A full-stack e-commerce marketplace built with **C++20**, **Drogon**, **SQLite3**, and **Vanilla JS**.

## Features
- Multi-role auth: Buyer, Seller, Admin (session-based)
- Product catalog with search, filter, pagination
- Shopping cart and checkout
- Order management with status tracking
- Product reviews and ratings
- Seller dashboard
- Admin panel with stats

## Tech Stack
| Layer | Technology |
|---|---|
| Language | C++20 |
| Framework | Drogon |
| Database | SQLite 3 |
| Crypto | OpenSSL SHA-256 |
| Build | CMake 3.20+ + vcpkg |
| Frontend | Vanilla HTML/CSS/JS |
| Testing | Catch2 v3 |
| CI | GitHub Actions |

## Building on Windows

### Prerequisites
- Visual Studio 2022 (with C++ Desktop Development workload)
- CMake 3.20+
- vcpkg at `C:\Users\lenovo\Documents\Default Project\vcpkg`

### Setup

```powershell
# 1. Navigate to project
cd "C:\Users\lenovo\Desktop\Blessi_MartNew"

# 2. Install all dependencies (reads vcpkg.json manifest automatically)
& "C:\Users\lenovo\Documents\Default Project\vcpkg\vcpkg.exe" install

# 3. Configure CMake  (note: path has a space, so quote it)
cmake -B build -DCMAKE_BUILD_TYPE=Release `
  "-DCMAKE_TOOLCHAIN_FILE=C:/Users/lenovo/Documents/Default Project/vcpkg/scripts/buildsystems/vcpkg.cmake"

# 4. Build
cmake --build build --config Release

# 5. Run tests
cd build
ctest -C Release --output-on-failure
cd ..

# 6. Run the server
.\build\Release\blessimart.exe
```

Then open **http://localhost:8080** in your browser.

> **Tip:** You can also set the toolchain file once in your `CMakeUserPresets.json` or VS Code
> `cmake.configureArgs` setting so you don't need to pass it every time.

## API Reference

### Auth
| Method | URL | Auth | Description |
|---|---|---|---|
| POST | /api/auth/register | No | Register new account |
| POST | /api/auth/login | No | Login |
| POST | /api/auth/logout | Yes | Logout |
| GET | /api/auth/me | Yes | Get current user |

### Products
| Method | URL | Auth | Description |
|---|---|---|---|
| GET | /api/products | No | List products (paginated) |
| GET | /api/products/{id} | No | Get single product |
| POST | /api/products | Seller/Admin | Create product |
| PUT | /api/products/{id} | Owner/Admin | Update product |
| DELETE | /api/products/{id} | Owner/Admin | Delete product |

### Cart
| Method | URL | Auth | Description |
|---|---|---|---|
| GET | /api/cart | Buyer | View cart |
| POST | /api/cart | Buyer | Add/update item |
| DELETE | /api/cart/{productId} | Buyer | Remove item |
| DELETE | /api/cart/clear | Buyer | Clear cart |

### Orders
| Method | URL | Auth | Description |
|---|---|---|---|
| POST | /api/orders | Buyer | Checkout |
| GET | /api/orders | Buyer/Admin | List orders |
| GET | /api/orders/{id} | Owner/Admin | Order detail |
| GET | /api/seller/orders | Seller | Seller's orders |
| PUT | /api/orders/{id}/status | Seller/Admin | Update status |

### Categories
| Method | URL | Auth | Description |
|---|---|---|---|
| GET | /api/categories | No | List categories |
| POST | /api/categories | Admin | Create |
| PUT | /api/categories/{id} | Admin | Update |
| DELETE | /api/categories/{id} | Admin | Delete |

### Reviews
| Method | URL | Auth | Description |
|---|---|---|---|
| POST | /api/reviews | Buyer | Post review |
| GET | /api/products/{id}/reviews | No | List reviews |
| DELETE | /api/reviews/{id} | Owner/Admin | Delete review |

### Admin
| Method | URL | Auth | Description |
|---|---|---|---|
| GET | /api/admin/users | Admin | List users |
| PUT | /api/admin/users/{id}/status | Admin | Suspend/activate |
| GET | /api/admin/stats | Admin | Platform stats |

## Project Structure
```
Blessi_MartNew/
├── CMakeLists.txt
├── vcpkg.json
├── schema.sql
├── README.md
├── .gitignore
├── config/
│   └── config.json
├── src/
│   ├── main.cpp
│   ├── db/
│   │   ├── Database.h
│   │   └── Database.cpp
│   ├── utils/
│   │   ├── Crypto.h
│   │   ├── Crypto.cpp
│   │   ├── JsonHelper.h
│   │   └── SessionHelper.h
│   └── controllers/
│       ├── AuthController.h/.cpp
│       ├── CategoryController.h/.cpp
│       ├── ProductController.h/.cpp
│       ├── CartController.h/.cpp
│       ├── OrderController.h/.cpp
│       ├── ReviewController.h/.cpp
│       └── AdminController.h/.cpp
├── frontend/
│   ├── index.html
│   ├── css/style.css
│   └── js/
│       ├── app.js
│       ├── auth.js
│       ├── products.js
│       ├── product-detail.js
│       ├── cart.js
│       ├── orders.js
│       ├── seller.js
│       └── admin.js
└── tests/
    ├── CMakeLists.txt
    ├── test_crypto.cpp
    ├── test_db.cpp
    └── test_helpers.cpp
```

## Default Admin Account
Insert via SQLite on first run — see schema for setup.

## License
MIT
