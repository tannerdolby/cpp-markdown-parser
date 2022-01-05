# mdparse
A Markdown parser written in C++ to convert input Markdown files to HTML. The parser supports setting element attributes inline within Markdown files for certain elements. Still a WIP so stay tuned :)

## Example
The following Markdown:

```md
# Hello World! [class=greeting]

Happy parsing.

[Browse the web](https://google.com)

![A cat with paws on toy sized MacBook Pro laptop pretending to type while sitting on owners lap](https://bukk.it/air.jpg)
```

will be transformed to:

```html
<h1 class="greeting">Hello World!</h1>

<p>Happy parsing.</p>

<a href="https://google.com">Browse the web</a>

<img src="https://bukk.it/air.jpg"  alt="A cat with paws on toy sized MacBook Pro laptop pretending to type while sitting on owners lap" />
```

## Usage
Provide a input Markdown file e.g. `test.md` and run the utility to output an HTML file.

todo: add example

## Resources
- [regex101](https://regex101.com)
- [markdown-guide](https://about.gitlab.com/handbook/markdown-guide/)