#pragma once
#ifndef _Base64_H__
#define _Base64_H__

char *base64_encode(const char *input, const size_t length, char *output);
char *base64_decode(const char *input, char *output);

#endif