# Implementation Plan: Pullar Landing Website (Astro + Cloudflare Pages)

## Goal Description
Create a new GitHub repository named `pullar-website` containing a minimalist, ultra-fast landing website built with **Astro** for the **Pullar** video and audio downloader ecosystem. The website will showcase both the **Windows** and **Android** applications, feature prominent direct download buttons for each platform, use the absolute minimum necessary text, adhere strictly to the custom 4-color palette, and contain zero emojis. The repository will be structured and optimized for direct one-click deployment to **Cloudflare Pages**.

---

## User Review Required

> [!IMPORTANT]
> **Repository Location**: The new repository will be created locally at `D:\My Projects 14 aug\antigravity cli\Rust\pullar-website` and pushed to GitHub as `1abdullahr1/pullar-website`.

> [!NOTE]
> **Minimalist Copywriting**: Following your requirement to use the "maximum possible least text", the website will feature clean, high-impact typography without long paragraphs. Features will be presented as concise punchy metrics (e.g., "16x Speed", "4K Video & MP3", "Built-in Player", "Batch Playlists").

> [!NOTE]
> **Zero Emojis Enforced**: Crisp, modern SVG icons (Windows, Android, Download Arrow, Media Player, Speed, GitHub) will be used throughout; no emojis will be used anywhere in the codebase or UI.

---

## Open Questions

> [!IMPORTANT]
> **GitHub Repository Visibility**: Should `1abdullahr1/pullar-website` be created as **Public** or **Private**? (Public is recommended for easy Cloudflare Pages linking).

---

## Proposed Architecture & Design

### Color Palette Specification
- **Coffee Bean (`#1a110f`)**: Deep dark canvas, background, base cards.
- **Light Apricot (`#ffebc2`)**: Primary headers, prominent typography, high-contrast badges.
- **Toffee Brown (`#945e38`)**: Borders, subtle card glow, secondary badges, dividers.
- **Maya Blue (`#7cc6fe`)**: Primary download buttons, active interactive states, accent glows.

### Cloudflare Pages Deployment Config
- **Framework**: Astro
- **Build Command**: `npm run build`
- **Output Directory**: `dist`
- **Node.js Compatibility**: `NODE_VERSION=20` or higher

---

## Proposed Project Structure

```
pullar-website/
├── .github/
│   └── workflows/
│       └── deploy.yml              # Optional Cloudflare Pages CI check
├── public/
│   ├── favicon.svg                 # SVG favicon matching app logo
│   └── app-logo.png                # High-res Pullar icon
├── src/
│   ├── components/
│   │   ├── Header.astro            # Minimalist navigation (Brand + GitHub link)
│   │   ├── Hero.astro              # Headline + Slogan + Quick Stats
│   │   ├── DownloadSection.astro   # Dual-platform download cards (Windows & Android)
│   │   ├── FeatureStrip.astro      # Minimalist 4-pill feature highlight
│   │   └── Footer.astro            # Lightweight footer & open-source license
│   ├── layouts/
│   │   └── Layout.astro            # Global layout, meta tags, font & palette CSS
│   └── pages/
│       └── index.astro             # Single-page high-converting landing page
├── astro.config.mjs                # Astro configuration
├── package.json                    # Dependencies & build scripts
├── tsconfig.json                   # TypeScript configuration
├── README.md                       # Documentation & Cloudflare Pages instructions
└── .gitignore                      # Git ignore rules for node_modules & dist
```

---

## Proposed Component Breakdown

### 1. Build & Dependency Setup
#### [NEW] `package.json`
- Dependencies: `astro: ^4.16.0` (or latest stable Astro 5.x)
- Scripts: `dev`, `start`, `build`, `preview`

#### [NEW] `astro.config.mjs`
- Output: `static` (optimized for Cloudflare Pages global CDN)

#### [NEW] `src/layouts/Layout.astro`
- Global styles injecting custom CSS variables:
  ```css
  :root {
    --color-coffee-bean: #1a110f;
    --color-light-apricot: #ffebc2;
    --color-toffee-brown: #945e38;
    --color-maya-blue: #7cc6fe;
    --color-surface-dark: #241816;
    --color-surface-card: #2b1d1a;
    --color-surface-border: #452d26;
  }
  ```
- Meta tags for SEO, social previews, Open Graph, and mobile viewport optimization.

### 2. UI Components
#### [NEW] `src/components/Header.astro`
- Left: Pullar icon + "Pullar" title in Maya Blue.
- Right: GitHub repository link with clean SVG icon.

#### [NEW] `src/components/Hero.astro`
- Title: **Pullar**
- Slogan: **Don't just watch—pull it**
- Minimalist description: One sentence defining the tool ("Fast, segmented video and audio downloader for Windows and Android.").

#### [NEW] `src/components/DownloadSection.astro`
- Two side-by-side cards:
  1. **Windows Card**:
     - Windows SVG icon.
     - Title: "Pullar for Windows".
     - Specs: "Windows 10 / 11 (x64) • Qt 6 & C++".
     - Primary Button: "Download Installer (.exe)" in Maya Blue (`#7cc6fe`).
     - Secondary Button: "Portable Bundle (.zip)" in Toffee Brown outline.
  2. **Android Card**:
     - Android SVG icon.
     - Title: "Pullar for Android".
     - Specs: "Android 7.0+ (ARM64 / x86_64) • Kotlin & Jetpack Compose".
     - Primary Button: "Download APK (.apk)" in Maya Blue (`#7cc6fe`).

#### [NEW] `src/components/FeatureStrip.astro`
- 4 ultra-concise pills (least possible text):
  - "16x Segmented Speed"
  - "Built-in Media Player"
  - "Full Playlist Support"
  - "Free & Open Source"

#### [NEW] `src/components/Footer.astro`
- Minimalist footer with MIT license, GitHub repo link, and zero emojis.

---

## Verification Plan

### Automated Verification
1. Run `npm install` inside the `pullar-website` directory.
2. Run `npm run build` to verify clean static site generation into `dist/`.
3. Verify that `dist/index.html` and assets are produced with 0 errors.

### Code & Visual Audit
1. Verify strict zero emojis across all generated code, markdown, and HTML.
2. Verify exact color palette matches `#1a110f`, `#ffebc2`, `#945e38`, and `#7cc6fe`.
3. Check responsive design on mobile and desktop viewports.
4. Verify git initialization and push to `1abdullahr1/pullar-website`.
