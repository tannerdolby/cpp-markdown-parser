# mdparse
A Markdown parser written in C++ to convert Markdown files to HTML. The parser supports setting element attributes inline within Markdown files for certain elements. docs are in progress

## Example
The following Markdown:

```md
# Hello World! [class=greeting]

Happy parsing.

[Browse the web](https://google.com)

![A cat with paws on toy sized MacBook Pro laptop pretending to type while sitting on owners lap](https://bukk.it/air.jpg)

- i'm blue [style=color: blue;]
- foo
- fizz

1. item one
2. item two [class=item-two]
3. item three
```

will be transformed to:

```html
<h1 class="greeting">Hello World!</h1>

<p>Happy parsing.</p>

<p><a href="https://google.com">Browse the web</a></p>

<img src="https://bukk.it/air.jpg"  alt="A cat with paws on toy sized MacBook Pro laptop pretending to type while sitting on owners lap" />

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

## Usage
Provide a input Markdown file path i.e. `./src/test-file.md` and the utility will generate and the resultant HTML file at the given output path. 

### Required Parameters
1. Input file path `char*`
2. Output file path `char*`

Both of the above params are expected for every instance of a [`File`](https://github.com/tannerdolby/cpp-markdown-parser/blob/a59572f8cdd07ade36cc13222a07d3494ae00573/parser.cpp#L25-L42). Reading a single file or multiple files at once is supported Support for reading a single file or multiple files.

## Resources
- [regex101](https://regex101.com)
- [markdown-guide](https://about.gitlab.com/handbook/markdown-guide/)