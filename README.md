# mdparse
Markdown parser written in C++ to convert Markdown files to HTML. Working on adhering to the CommonMark spec. This parser supports setting element attributes inline within Markdown files for certain elements.

## Usage
Provide an input Markdown file path and the parser will generate a resultant HTML file at the given output path.

Two command line arguments must be specified when executing the main file:

1. Input file path
2. Output file path

```
make build
./main input-file.md output-file.html
```

## What does it do?
The parser will accept an input Markdown file:

```md
# Hello World! [class=greeting]

Happy parsing.

[Browse the web](https://google.com)

![A cat with paws on toy sized MacBook Pro laptop pretending to type](https://bukk.it/air.jpg)

Hello, World [test](t.com) and this is another [link](f.com) with more text after it.

- i'm blue [style=color: blue;]
- foo
- fizz

1. item one
2. item two [class=item-two]
3. item three
```

and will transform the Markdown to HTML:

```html
<h1 class="greeting">Hello World!</h1>

<p>Happy parsing.</p>

<p><a href="https://google.com">Browse the web</a></p>

<img src="https://bukk.it/air.jpg"  alt="A cat with paws on toy sized MacBook Pro laptop pretending to type" />

<p>Hello, World <a href="t.com">test</a> and this is another <a href="f.com">link</a> with more text after it.</p>

<ul>
    <li style="color: blue;">i'm blue</li>
    <li>foo</li>
    <li>fizz</li>
</ul>

<ol>
    <li>item one</li>
    <li class="item-two">item two</li>
    <li>item three</li>
</ol>
```

## Resources
- [CommonMark Spec](https://github.com/commonmark/commonmark-spec)
- [regex101](https://regex101.com)
- [markdown-guide](https://about.gitlab.com/handbook/markdown-guide/)