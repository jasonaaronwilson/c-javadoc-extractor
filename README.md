# C Javadoc Extractor

This tool extracts "documentation" comments from source files and
"assembles" them into markdown files mirroring the structure of the
source code but without the actual "code". github and other tools
present such files in an enhanced manner to make them more readable
than just plain text (and hyperlinks work to make thing more
navigable, etc.).

In some sense this tool is the dual of a compiler which completely
ignores your comments when it compiles your source code since this
tool in turn *completely* ignores the code.

## Wait, what? Why?

Yes, "*completely* ignores the code". The ridicuously simple idea is
that we are just putting the code and documentation near each other
and won't try to parse or verify that the documentation comment
actually matches the code. (For that, I believe LLMs will eventually
be the standard instead of parsing the code with a standard parser.)

## Tags

"@file", "@function", "@type", "@global", "@constants" (aka
enumerations or #define blocks), and "@macro" are all language
agnostic tags that can be used to help the documentation extractor
understand your documentation intent without actually trying to parse
your source code (and allows sorting, perhaps it's only real purpose).

All tags appears immediately after the start of a documentation
comment so for C that might look like this:

```
/**
 * @function string_pad_left
 *
 * Prefix a string with left padding (if necessary) to make it at
 * least N bytes long and always return a newly allocated string.
 */
char* string_pad_left(const char* str, int count, char ch) {
  ...
}
```

When a tag is used but not understood, it will get it's own section,
this will not be an error.

The name after the tag should be whitespace delimited, it need not be
a legal identifier in any particular language.

## Ordering

Ordering is based on tags and then what appears after tags (using
simple ASCII based alphabetization for the early versions).

## WARNING

This is how the tool *will* work and I've not written any code yet!
This is in part a stress test for my C library extension called
c-armyknife-lib.

## "Unix" Usage

The simplest way to to use the tool is to install "tcc" into /usr/bin
(and then no explicit compilation is required, just pretend it's a
shell script or a precompiled binary):

```
  c-javadoc-extractor --output-dir=src-doc *.[ch]
```

Or if you have a "complex" directory structure:

```
  find . -name '*.[ch]' | xargs c-javadoc-extractor --output-dir=src-doc
```

If someone wants to make a package for debian or arch then I would be
delighted. [^2]

## Usage Windows

TBD. (I believe tcc will output a Windows binary and I may puruse that
first. I personally don't have any funds to purchase Microsoft's
tools).

## Output Structure/Format

The output structure is one or more files placed into the directory
specified by `--output-dir=` (or the current directory if
`--output-dir` is not specified, **not recommended** (unless you only
have a couple of files in your repo and don't expect it to grow in
which case you may want to rename "index.md" to "README.md" using the
option --index-file-name=README.md").

A single root file called `index.md` is always created (unless
`--no-index=true` or `--index-file-name=xyz` are used) which will
contain links to all other generated files (in a nice
alphabetical/hierarchical order (according to strcmp, so possibly much
better in English and may depend on the C locale --- I admit we should
improve on that).

All generated files are named the same as each input file except the
extension (i.e., ".c" or ".h") is replace with ".md". If there are
javadoc comments in `foo.c` and `foo.h` (in the same directory), then
they are merged.

My style would be to only document in a C file unless the entity only
exists in a header file. This is where the person writing code is
mostly likely to see the documention and therefore either abide by it
or enhance it since it is so close to where they are editing.

If you rename your source files, then as long as your "src-doc"
directory doesn't contain your own hand-generated artifacts (which is
why it was named that instead of "doc" or "documentation" by the way),
simply erase it before generation if you are using a version control
system like git which is both smart enough to realize when file
contents haven't changed and when they have probably moved or been
deleted.

## Comparision with Doyxgen

Doxygen was designed to generate HTML output by default (as far as I
can tell). I asked "bard" and it says the "-m" or `OUTPUT_FORMAT =
MARKDOWN` options enable Doxygen to also produce markdown format
instead of HTML (and this is actually a real thing, see
https://www.doxygen.nl/manual/markdown.html). However, I think these
options just tell Doxygen that that your *documentation comments* are
markdown not that it should itself *generate* markdown. It looks like
there are tools such as Moxygen that will convert an XML file that
Doyxgen can produce to markdown. I have not tried that tool.

When Doxygen emits HTML, it certainly does much more than this tool
does (for example, it clearly invokes "dot" to create the output of
graphs of some sort) but I was actually a bit confused the first time
I used Doxygen (I think because the default HTML is styled badly and
may not fit on my laptop screen all that well - I don't think it
employs so called "responsive" HTML (though maybe someone else solved
that via CSS you can just include into you Doyxgen somehow)). I was
just looking at doxygen manual itself on a bigger screen, and while it
is not typographically a work of art, it's OK - definitely not
"repsonsive" - a column or two should disappear as the width of the
window is decreased instead the text part just keeps shrinking).

Doxygen supports many more formats for documentation comments than
just "javadoc" though consistency matters and I'm pretty confortable
with the Javadoc comment syntax myself for C derived languages that
accept "/*" and "*/" as comment delimiters [^2].

## Incremental Usage

`--no-index=true` provides some capabilities for incremental update
though the goal is to be very fast and I develop everything on a
1.3GHz processor Pixelbook so a faster machine should make very light
work of large code bases.

## Status

Well obviously this has actually been written yet. I'm on it.

### Footnotes

[^1]: it could be compiled with gcc or clang and probably be 2X faster
and more and be more convenient as well (since these typically appear
in the default path and gcc and clang spend more time optimizing code
than tcc).

[^2]: I'm not a fan now of putting HTML style markdown in Javadoc
although being "markdown" means this is legal and possibly desireable
if you want to do something markdown can't handle. As an author and a
reader, I just want comments to be pretty.
