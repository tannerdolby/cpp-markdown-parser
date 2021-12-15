# mdparse
A Markdown parser written in C++ to convert input Markdown files to HTML. The parser supports setting element attributes inline within Markdown files for certain elements. Still a WIP so stay tuned :)

## Example
The following Markdown:

```md
# Hello World! [class=greeting]

Happy parsing.

![A cat with paws on toy sized MacBook Pro laptop pretending to type while sitting on owners lap](https://bukk.it/air.jpg)
```

will be transformed to:

```html
<h1 class="greeting">Hello World!</h1>

<p>Happy parsing.</p>

<img src="https://bukk.it/air.jpg"  alt="A cat with paws on toy sized MacBook Pro laptop pretending to type while sitting on owners lap" />
```

![Screenshot of generated HTML file displayed in web browser](https://user-images.githubusercontent.com/48612525/146151640-a99248c0-6f1d-41d2-8dd6-9bb7af99f1c2.png)

## Usage
Provide a input Markdown file e.g. `test.md` and run the utility to output an HTML file.

todo: add example

## Resources
[regex101](https://regex101.com)
[markdown-guide](https://about.gitlab.com/handbook/markdown-guide/)