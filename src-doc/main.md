# @file main.c

Extract and organize "documentation comments".
 
## @structure buffer_range_t

Holds a simple pair of a start and an end index which describes the
entire comment. Since the comment will likely have internal
characters that must be removed anyways to create markdown, there's
no real advantage to trying to strip off any leading and trailing
parts of the comment at this point and it makes the code a bit
cleaner not to do so.
 
## @structure output_file_t

Represents an output markdown file we are creating. We keep track
of all the input files which map to the same makrdown file (for
example a ".c" and a ".h" can both contribute to the output
markdown file). More importantly we keep track of all of the
comments which match the documentation comment syntax which we call
"fragments".
 
## @function comment_to_markdown

Convert a C style documentation comment to it's plain markdown
equivalent.

(Currently we don't strip off or convert the tag which can probably
make all of the documentation look much better.)
 
## @function extract_documentation_comments

Read filename into memory and scan for all of the documentation
comments. Extract each comment as a C string and add as a fragment
to an existing "output file". (If we changed buffer_range_t to
include the buffer, we could make the code possibly more efficient
but that might it more confusing).
 
## @function make_output_file

Allocate an output_file_t and fill in the reasonable initial
values.
 
## @function next_comment

Return the next comment (as a buffer_range_t) in the buffer. This
must occur at or after the end of the passed in range (i.e., the
previous comment).
 
## @function output_markdown_file

Output a single makrdown file.
 
## @function output_markdown_file_fragment

Put matching markdown fragments into `output_buffer.`
 
## @function output_markdown_files

Output all of the markdown files that we know about (except the
"index.md" or "README.md" which is generated afterwards if
requested).
 
## @function output_readme_markdown_file

Output an "index" markdown file to tie everything together.
 
