#ifndef B2H_H__
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <stddef.h>
/* 'name' is not the file path. b2h does not read files, only strings. Does not take a size pointer for the output because realistically if it is necessary to read length multiple times its fast enough to strlen the output once and save it somewhere else. */
unsigned char *b2h(char *name, unsigned char *input, size_t input_length);
#endif

#if defined(B2H_IMPLEMENTATION) || defined(B2H_STANDALONE)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

unsigned char *b2h(char *name, unsigned char *input, size_t input_length)
{
	const char *fmt64 = "unsigned long long %s_len = %lluULL;unsigned char %s[]={";
	const char *fmt32 = "unsigned long %s_len = %luUL;unsigned char %s[]={";
	char *name_str = NULL, *fmt_str = NULL, *out_str = NULL;
	size_t hex_len = input_length * strlen("0x00,"), i;
	char hex_buf[7] = {0};
	int fmt_len = 0;
	

	/* assemble name identifier str */
	if(!(name_str = calloc(1, strlen((const char *)name) + 1 + 1))) /* extra + 1 because the identifier first char may need to be modified */
	{
		fprintf(stderr, "could not allocate filtered name string buffer\n");
		goto error;
	}

	for(i = 0; i < strlen(name); i++)
	{
		/* make sure identifier doesnt start with a number */
		if(!i && isdigit(name[i]))
		{
			strcat(name_str, "n");/* just prepend an n for number or something */
			fprintf(stderr, "warning: prepending n to identifier name because the filename started with a number\n");
		}

		if(isalnum(name[i]))
			strncat(name_str, &name[i], 1);
		else
			strcat(name_str, "_");
	}


	/* assemble format string */
	if((fmt_len = snprintf(fmt_str, 0, (input_length > ULONG_MAX) ? fmt64 : fmt32, name_str, input_length, name_str)) <= 0)
	{
		fprintf(stderr, "could not calc fmt string\n");
		goto error;
	}

	if(!(fmt_str = calloc(1, (size_t)fmt_len + 1)))
	{
		fprintf(stderr, "could not allocate fmt string buffer\n");
		goto error;
	}

	if(snprintf(fmt_str, (size_t)fmt_len + 1, (input_length > ULONG_MAX) ? fmt64 : fmt32, name_str, input_length, name_str) <= 0)
	{
		fprintf(stderr, "could not write fmt string\n");
		goto error;
	}


	/* assemble output string */
	if(!(out_str = calloc(1, hex_len + (size_t)fmt_len + strlen("0x00};\n") + 1)))
	{
		fprintf(stderr, "could not allocate hex string buffer\n");
		goto error;
	}

	/* prefix */
	strcat(out_str, fmt_str);
	/* hex */
	for(i = 0; i < input_length; i++)
	{
		snprintf(hex_buf, sizeof(hex_buf) - 1, "0x%x,", input[i]);
		strncat(out_str, hex_buf, 5);
	}
	/* postfix */
	strcat(out_str, "0x00};\n");


	free(name_str);
	free(fmt_str);

	return (unsigned char *)out_str;
error:
	if(name_str) free(name_str);
	if(fmt_str) free(fmt_str);
	if(out_str) free(out_str);

	return NULL;
}

#ifdef B2H_STANDALONE
int main(int argc, char **argv)
{
	FILE *fin = NULL, *fout = NULL;
	unsigned char *input = NULL, *output = NULL;
	size_t input_len = 0, output_len = 0;


	if(argc != 3)
	{
		printf("usage:\nb2h <file-in> <file-out>\n");
		goto error;
	}


	if(!(fin = fopen(argv[1], "rb")))
	{
		fprintf(stderr, "could no open file '%s'\n", argv[1]);
		goto error;
	}

	if(!(fout = fopen(argv[2], "wb")))
	{
		fprintf(stderr, "could not create or write file '%s'\n", argv[2]);
		goto error;
	}


	fseek(fin, 0, SEEK_END);
	input_len = (size_t)ftell(fin);
	fseek(fin, 0, SEEK_SET);

	if(!(input = calloc(1, input_len)))
	{
		fprintf(stderr, "could not allocate file input string\n");
		goto error;
	}

	if(fread(input, sizeof(unsigned char), input_len, fin) != input_len)
	{
		fprintf(stderr, "could not read entire input file\n");
		goto error;
	}

	if(!(output = b2h(argv[1], input, input_len)))
	{
		fprintf(stderr, "could not create output data\n");
		goto error;
	}

	output_len = strlen((const char *)output);
	if(fwrite(output, sizeof(unsigned char), output_len, fout) != output_len)
	{
		fprintf(stderr, "could not write output contents\n");
		goto error;
	}


	/* im aware this is silly when the program is exitting right now. Makes it easier to use tools like valgrind to double check everything */
	free(input);
	free(output);
	fclose(fin);
	fclose(fout);


	return 0;
error:
	if(fin) fclose(fin);
	if(fout) fclose(fout);
	if(input) free(input);

	return 1;
}
#endif /* standalone */
#endif /* implementation */
