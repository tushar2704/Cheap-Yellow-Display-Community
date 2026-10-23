# CYD Flasher — Vercel Deployment

A lightweight, standalone **1-click in-browser ESP32 firmware flasher** for the Cheap Yellow Display community.

- ✅ Deploy to Vercel in 30 seconds (zero build step)
- ✅ Firmware binaries served from GitHub Pages (always in sync with the repo)
- ✅ Donate modal shows once per session before first flash
- ✅ Web Serial detection with browser compatibility message
- ✅ Filter + search across all 13 projects
- ✅ Manual `.BIN` download for every project

## Deploy to Vercel

[![Deploy with Vercel](https://vercel.com/button)](https://vercel.com/new/clone?repository-url=https://github.com/tushar2704/Cheap-Yellow-Display-Community&root=cyd-flasher)

### Manual steps
1. Go to [vercel.com/new](https://vercel.com/new)
2. Import `tushar2704/Cheap-Yellow-Display-Community`
3. Set **Root Directory** → `cyd-flasher`
4. Leave Build Command blank (pure static)
5. Click Deploy ✓

## Update firmware list

Edit the `FW` array inside `index.html`. Each entry looks like:

```js
{
  id:       'hello-cyd',
  tier:     'B',                    // B = Beginner, I = Intermediate, A = Advanced
  name:     '01 — Hello CYD',
  desc:     'Short description',
  tags:     ['Tag1','Tag2'],
  bin:      '01-hello-cyd.bin',     // filename in /flasher/binaries/ on GitHub Pages
  manifest: 'hello-cyd.json'        // filename in /flasher/manifests/ on GitHub Pages
}
```

## Donate links

Update these two URLs in `index.html`:

| Variable | Location | Default |
|---|---|---|
| `DONATE_URL` | JS config block | `https://github.com/sponsors/tushar2704` |
| Buy Me a Coffee | `btn-donate` anchor | `https://buymeacoffee.com/tushar2704` |

## Credits

- Tech & Humor: [@IamNotIntoTech](https://youtube.com/@IamNotIntoTech)
- 3D Design & Printing: [@Makerrdesigns](https://youtube.com/@Makerrdesigns)
- Author: [Tushar Aggarwal (@tushar2704)](https://github.com/tushar2704)
