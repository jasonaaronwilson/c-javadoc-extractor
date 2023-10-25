
///
/// Make an array of uint64 and add 16 elements to it.
///

#include <stdlib.h>

#define LOGGER_DEFAULT_LEVEL LOGGER_INFO
#define C_ARMYKNIFE_LIB_IMPL
#include "../c-armyknife-lib/c-armyknife-lib.h"

/**
 * Read filename into memory and scan for all of the documentation
 * comments. Extract each comment, remove the C style comment stuff,
 * and then add each comment to the output file.
 */
string_hashtable_t *extract_documentation_comments(string_hashtable_t *output,
                                                   char *filename) {
  log_info("Reading %s\n", filename);
  buffer_t *source_file = make_buffer(1);
  source_file = buffer_append_file_contents(source_file, filename);

  int position = 0;
  while (position < source_file->length) {
    if (buffer_get(source_file, position) == '/' &&
        buffer_get(source_file, position + 1) == '*' &&
        buffer_get(source_file, position + 2) == '*') {
      int start_position = position;
      int end_position = start_position;
      position += 3;
      while (position < source_file->length) {
        if (buffer_get(source_file, position) == '*' &&
            buffer_get(source_file, position + 1) == '/') {
          end_position = position;
          position += 2;
          break;
        } else {
          position++;
        }
      }

      log_info("javadoc comment found at %d,%d\n", start_position,
               end_position);

    } else {
      position++;
    }
  }

  return output;
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

  string_hashtable_t *output = make_string_hashtable(16);
  for (int i = 0; i < args_and_files.files->length; i++) {
    char *filename = value_array_get(args_and_files.files, i).str;
    output = extract_documentation_comments(output, filename);
  }

  exit(0);
}
