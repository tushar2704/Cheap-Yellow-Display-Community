# 🤝 Contributing to Cheap Yellow Display Community

First off, **thank you** for considering contributing to the CYD Community! Whether you're sharing a project, fixing a bug, improving documentation, or designing a 3D enclosure — every contribution makes this community better.

---

## 📋 Table of Contents

- [Ways to Contribute](#ways-to-contribute)
- [Submitting a Project](#submitting-a-project)
- [Reporting Bugs](#reporting-bugs)
- [Improving Documentation](#improving-documentation)
- [3D Enclosure Designs](#3d-enclosure-designs)
- [Code Style Guidelines](#code-style-guidelines)
- [Pull Request Process](#pull-request-process)

---

## Ways to Contribute

| Type | Description | Difficulty |
|------|-------------|------------|
| 🐛 Bug Report | Found a bug? Let us know! | Easy |
| 📝 Documentation | Fix typos, improve guides, add examples | Easy |
| 🔧 Project | Submit a new CYD project | Medium |
| 🖨️ Enclosure | Design a 3D printable case | Medium |
| 💡 Feature Request | Suggest a new project or feature | Easy |

---

## Submitting a Project

### Project Structure

Your project **must** follow this directory structure:

```
projects/{difficulty}/{NN}-{project-name}/
├── README.md            # Required
├── {project-name}.ino   # Required (Arduino sketch)
├── platformio.ini       # Required
├── src/                 # Optional
├── assets/              # Screenshots/GIFs (recommended)
└── enclosure/           # 3D files (optional)
```

### Project README Requirements

Every project README must include:

1. **Title** and one-line description
2. **Screenshot or GIF** of the project running
3. **Requirements** (hardware + software)
4. **Installation instructions** (Arduino IDE + PlatformIO)
5. **Supported CYD variants** (which models work)
6. **License** reference

### Testing

- Your code must **compile without errors** on at least one CYD variant
- Test on real hardware if possible
- Include which CYD model you tested on

---

## Reporting Bugs

Use the [Bug Report template](.github/ISSUE_TEMPLATE/bug_report.md) and include:

1. Which CYD model you're using (e.g., ESP32-2432S028R)
2. Your development environment (Arduino IDE version or PlatformIO version)
3. What you expected to happen
4. What actually happened
5. Steps to reproduce
6. Any error messages or screenshots

---

## Improving Documentation

Documentation contributions are **always welcome**! This includes:

- Fixing typos or grammar
- Adding missing information to guides
- Improving code examples
- Translating documentation to other languages
- Adding pin maps or diagrams

For documentation changes, simply submit a Pull Request.

---

## 3D Enclosure Designs

We love 3D printable enclosure submissions! Please include:

- **STL file** (for printing)
- **STEP file** (for editing)
- **README** with print settings (layer height, infill, supports)
- **Photo** of the printed enclosure (if possible)
- Which CYD variant it fits

---

## Code Style Guidelines

### Arduino/C++
- Use **4 spaces** for indentation (no tabs)
- Use descriptive variable names
- Comment non-obvious code
- Include a file header comment with project name and license
- Use `#define` for CYD variant selection (see [MASTER_INSTRUCTIONS.md](MASTER_INSTRUCTIONS.md))

### Documentation
- Use standard Markdown
- Include alt text on images
- Use relative links within the repo

---

## Pull Request Process

1. **Fork** the repository
2. **Create a branch** from `main` (`git checkout -b feat/my-project`)
3. **Make your changes** following the guidelines above
4. **Test** your code compiles
5. **Commit** with a descriptive message (see [commit convention](MASTER_INSTRUCTIONS.md#commit-message-convention))
6. **Push** to your fork
7. **Open a Pull Request** against `main`

### PR Checklist

- [ ] Code compiles without errors
- [ ] README follows the project template
- [ ] Screenshots are included (for projects)
- [ ] No unnecessary files committed
- [ ] Commit messages follow the convention

---

## 💬 Questions?

If you're unsure about anything, open an [Issue](../../issues) or start a [Discussion](../../discussions) — we're happy to help!

---

<p align="center">
  <b>Thank you for making the CYD community awesome! 🟡</b>
</p>
