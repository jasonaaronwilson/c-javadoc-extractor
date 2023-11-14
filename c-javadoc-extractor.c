/**
 * @file c-javadoc-extractor

 * This tool extracts documentation comments (assumed to be formatted
 * as markdown) and produces markdown files that mirror the structure
 * of your source code into a directory of your choice (we suggest
 * src-doc).
 *
 * Although we rely on "c-armyknife-lib" which is about 4K lines of
 * code including verbose documentation, this program is tiny by
 * modern standards and extremely fast.
 */

#include <stdlib.h>
#include <sys/types.h>

#define BUFFER_PRINTF_INITIAL_BUFFER_SIZE 32
#define LOGGER_DEFAULT_LEVEL LOGGER_INFO
#define C_ARMYKNIFE_LIB_IMPL
#include "../c-armyknife-lib/c-armyknife-lib.h"

struct output_file_S {
  char *output_file_name;
  value_array_t *input_file_names;
  string_tree_t *fragments;
};

typedef struct output_file_S output_file_t;

output_file_t *make_output_file(char *output_file_name) {
  output_file_t *result = malloc_struct(output_file_t);
  result->output_file_name = output_file_name;
  result->input_file_names = make_value_array(1);
  result->fragments = NULL;
  return result;
}

// Look for first occurence of "." and assume we can just tack on "md"
// after that to produce the filename. This will only work if
// input_filename actually contains a "." and is actually the last
// occurence in the string.
char *source_file_to_output_file_name(char *input_filename) {
  char *substr = string_substring(input_filename, 0,
                                  string_index_of_char(input_filename, '.'));
  char *result = string_append(substr, ".md");
  free_bytes(substr);
  return result;
}

struct buffer_range_S {
  uint64_t start;
  uint64_t end;
};

typedef struct buffer_range_S buffer_range_t;

/**
 * @function next_comment
 *
 * Return the next comment (as a buffer_range_t) in the buffer. This
 * must always occurs at or after the end of the passed in range
 * (i.e., the previous comment).
 */
buffer_range_t next_comment(buffer_t *buffer, buffer_range_t range) {
  for (int position = range.end; (position < buffer->length - 2); position++) {
    if (buffer_get(buffer, position) == '/' &&
        buffer_get(buffer, position + 1) == '*' &&
        buffer_get(buffer, position + 2) == '*') {
      int start_position = position;
      int end_position = start_position;
      position += 3;
      while (position < buffer->length - 1) {
        if (buffer_get(buffer, position) == '*' &&
            buffer_get(buffer, position + 1) == '/') {
          end_position = position + 2;
          return (buffer_range_t){.start = start_position, .end = end_position};
        } else {
          position++;
        }
      }
      log_fatal(
          "The javadoc commented at position %s is not properly terminated.");
      fatal_error(ERROR_UKNOWN);
    }
  }
  return (buffer_range_t){.start = 0, .end = 0};
}

char *comment_to_markdown(char *comment) {
  uint64_t length = strlen(comment);
  log_info("comment length = %d\n", length);
  comment = string_substring(comment, 3, length - 2);
  value_array_t *lines = tokenize(comment, "\n");
  buffer_t *buffer = make_buffer(length);
  for (int i = 0; i < lines->length; i++) {
    char *line = value_array_get(lines, i).str;
    if (string_starts_with(line, " * ")) {
      line += 3;
    } else if (string_starts_with(line, " *")) {
      line += 2;
    }
    buffer = buffer_append_string(buffer, line);
    buffer = buffer_append_byte(buffer, '\n');
  }
  // TODO(jawilson): add some free_bytes
  return buffer_to_c_string(buffer);
}

/**
 * Read filename into memory and scan for all of the documentation
 * comments. Extract each comment, remove the C style comment stuff,
 * and then add each comment to the output file.
 */
string_hashtable_t *
extract_documentation_comments(string_hashtable_t *output_files,
                               char *filename) {
  log_info("Reading %s\n", filename);

  char *output_file_name = source_file_to_output_file_name(filename);
  output_file_t *output_file = NULL;

  value_result_t existing_output_file_result =
      string_ht_find(output_files, output_file_name);
  if (is_ok(existing_output_file_result)) {
    output_file = existing_output_file_result.ptr;
  } else {
    output_file = make_output_file(output_file_name);
    output_files = string_ht_insert(output_files, output_file_name,
                                    ptr_to_value(output_file));
  }

  value_array_add(output_file->input_file_names, str_to_value(filename));

  buffer_t *source_file = make_buffer(1);
  source_file = buffer_append_file_contents(source_file, filename);

  buffer_range_t comment_range = (buffer_range_t){.start = 0, .end = 0};
  while (1) {
    comment_range = next_comment(source_file, comment_range);
    if (comment_range.start == comment_range.end) {
      break;
    }
    log_info("javadoc comment found at [%d,%d)\n", comment_range.start,
             comment_range.end);
    char *comment =
        buffer_c_substring(source_file, comment_range.start, comment_range.end);
    // Eventually we could use something smaller for the key but for
    // now we just sort on the entire comment itself.
    output_file->fragments =
        string_tree_insert(output_file->fragments, comment,
                           str_to_value(comment_to_markdown(comment)));
  }

  log_info("Done reading %s\n", filename);

  return output_files;
}

// Output an "index" markdown file to tie everything together

void output_readme_markdown_file(string_hashtable_t *output_files,
                                 char *output_directory) {

  // We now desire output_files to be sorted but we can don't have to
  // implment that just yet.

  buffer_t *output_buffer = make_buffer(1024);

  output_buffer =
      buffer_append_string(output_buffer, "# Source Documentation Index\n\n");

  string_ht_foreach(output_files, output_file_name, fragment_value, {
    output_buffer = buffer_printf(output_buffer, "* [%s](%s)\n",
                                  output_file_name, output_file_name);
    output_buffer = buffer_append_string(output_buffer, "\n");
  });
  buffer_write_file(output_buffer,
                    string_append(output_directory, "README.md"));
}

void output_markdown_files(string_hashtable_t *output_files,
                           char *output_directory) {
  log_info("*** Starting output of markdown files ***");

  log_info("Making sure the output directory %s exists...", output_directory);
  {
    struct stat st;
    const char *directory_path = output_directory;
    if (stat(directory_path, &st) != 0) {
      // The directory does not exist, so create it.
      mkdir(directory_path, 0755);
    }
  }

  // While additional sorting may make sense, let's see how bad this
  // looks first.

  // clang-format off
  string_ht_foreach(output_files, output_filename, output_file_value, {
      log_info("Another output file... %s", output_filename);
      output_file_t* output_file = output_file_value.ptr;
      buffer_t* output_buffer = make_buffer(1024);
      string_tree_foreach(output_file->fragments, fragment_key, fragment_value, {
          char* fragment_text = fragment_value.ptr;
          fprintf(stderr, "-----Fragment (file=%s)-----\n%s-----(end fragment)-----\n\n", 
                  output_filename, 
                  fragment_text);
          if (string_starts_with(fragment_text, "@file")) {
            output_buffer = buffer_append_string(output_buffer, "# ");
          } else {
            output_buffer = buffer_append_string(output_buffer, "## ");
          }
          output_buffer = buffer_append_string(output_buffer, fragment_text);
        });
      // This is where we create and write an output file now that it
      // is complete.
      buffer_write_file(output_buffer, string_append(output_directory, output_filename));
    });
  // clang-format on

  output_readme_markdown_file(output_files, output_directory);

  log_info("Done outputting markdown files.");
}

/* ====================================================================== */

value_array_t *get_command_line_command_descriptors() { return NULL; }

value_array_t *get_command_line_flag_descriptors() {
  value_array_t *result = make_value_array(2);
  value_array_add(result, ptr_to_value(make_command_line_flag_descriptor(
                              "output-dir", command_line_flag_type_string,
                              "where to place the generated files")));
  return result;
}

command_line_parser_configuation_t *get_command_line_parser_config() {
  command_line_parser_configuation_t *config =
      malloc_struct(command_line_parser_configuation_t);
  config->program_name = "c-javadoc-extractor";
  config->program_description =
      "A simple program to extract markdown in javadoc style comments\n"
      "and create *markdown* files from it.";
  config->command_descriptors = get_command_line_command_descriptors();
  config->flag_descriptors = get_command_line_flag_descriptors();
  return config;
}

int main(int argc, char **argv) {
  logger_init();

  command_line_parse_result_t args_and_files =
      parse_command_line(argc, argv, get_command_line_parser_config());

  string_hashtable_t *files = make_string_hashtable(16);
  for (int i = 0; i < args_and_files.files->length; i++) {
    char *filename = value_array_get(args_and_files.files, i).str;
    files = extract_documentation_comments(files, filename);
  }

  // TODO(jawilson): hashtable find_or_default or something instead of
  // forcing "src-doc".
  output_markdown_files(files, "src-doc/");

  exit(0);
}
