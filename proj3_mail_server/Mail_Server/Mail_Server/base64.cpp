# include "stdafx.h"
# include "base64.h"
# include "Mail_ServerDlg.h"

//const char base64_index[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//
//
//char *base64_encode(const char *input, const size_t length, char *output)
//{
//	*output = '\0';
//	//如果输入字符串为空或指针无指向，则放回空字符串
//	if (input == NULL || output == NULL || length < 1) return output;
//
//	char *p_in = (char*)input;	//输入字符串的开始指针位置
//	char *p_out = (char*)output;;	//输出字符串的起始地址
//	char *p_in_end = (char*)input + length;	//输入字符串的结束位置
//	int  loop_count = 0;
//
//	// 0x30 -> 00110000  
//	// 0x3C -> 00111100  
//	// 0x3F -> 00111111  
//	//如果位数剩余不足3个字节,则跳出循环，每次处理3个字节，生成4个base64编码字符
//	while (p_in_end - p_in >= 3) {
//		*p_out++ = base64_index[(p_in[0] >> 2)];
//		*p_out++ = base64_index[((p_in[0] << 4) & 0x30) | (p_in[1] >> 4)];
//		*p_out++ = base64_index[((p_in[1] << 2) & 0x3C) | (p_in[2] >> 6)];
//		*p_out++ = base64_index[p_in[2] & 0x3F];
//		p_in += 3;
//	}
//	//处理最后3个字节以内的编码
//	if (p_in_end - p_in > 0) {
//		*p_out++ = base64_index[(p_in[0] >> 2)];
//		if (p_in_end - p_in == 2) {
//			*p_out++ = base64_index[((p_in[0] << 4) & 0x30) | (p_in[1] >> 4)];
//			*p_out++ = base64_index[(p_in[1] << 2) & 0x3C];
//			*p_out++ = '=';
//		}
//		else if (p_in_end - p_in == 1) {
//			*p_out++ = base64_index[(p_in[0] << 4) & 0x30];
//			*p_out++ = '=';
//			*p_out++ = '=';
//		}
//	}
//	*p_out = '\0';
//	return output;
//}
//
//int get_index(char a) {
//	if (a <= '9'&&a >= '0')
//		return 52 + a - '0';
//	else if (a <= 'Z'&&a >= 'A')
//		return a - 'A';
//	else if (a <= 'z'&&a >= 'a')
//		return a - 'a' + 26;
//}
//
//char *base64_decode(const char *input, char *output)
//{
//	output[0] = '\0';   //输出设为空
//
//	if (input == NULL || output == NULL)
//		return output;//输入输出没有设置指针，返回空
//	int input_len = strlen(input);
//
//
//	if (input_len < 4 || input_len % 4 != 0)		//如果base64编码字符串非4的倍数，说明编码串有错误，返回空
//		return output;
//	// 0xFC -> 11111100  
//	// 0x03 -> 00000011  
//	// 0xF0 -> 11110000  
//	// 0x0F -> 00001111  
//	// 0xC0 -> 11000000  
//	char *p = (char*)input;
//	char *p_out = output;
//	char *p_end = (char*)input + input_len;
//	for (; p < p_end; p += 4) {
//		*p_out++ = (((get_index(p[0]) << 2) & 0xFC) | (get_index(p[1]) >> 4) & 0x03);
//		*p_out++ = ((get_index(p[1]) << 4) & 0xF0) | ((get_index(p[2]) >> 2) & 0x0F);
//		*p_out++ = ((get_index(p[2]) << 6) & 0xC0) | (get_index(p[3]));
//	}
//	if (*(input + input_len - 2) == '=') {
//		*(p_out - 2) = '\0';
//	}
//	else if (*(input + input_len - 1) == '=') {
//		*(p_out - 1) = '\0';
//	}
//	return output;
//}
//// 16 * 16  
//const int base64_decode_map[256] = {
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0   - 15  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16  - 31  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, // 32  - 47  
//	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, // 48  - 63  
//	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, // 64  - 79  
//	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, // 80  - 95  
//	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96  - 111  
//	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 112 - 127  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128 - 143  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 144 - 159   
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160 - 175  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 176 - 191  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 192 - 207  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 208 - 223  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 224 - 239  
//	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 240 - 255  
//};

const char base64_index[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


int get_index(char a) {
	if (a <= '9'&&a >= '0')
		return 52 + a - '0';
	else if (a <= 'Z'&&a >= 'A')
		return a - 'A';
	else if (a <= 'z'&&a >= 'a')
		return a - 'a' + 26;
	else if (a == '/')
		return  63;
	else if (a == '+')
		return 62;
	return 0;
}

char *base64_encode(const char *input, const size_t length, char *output)
{
	*output = '\0';
	if (input == NULL || length < 1) return output;

	char *p = (char*)input;
	char *p_dst = (char*)output;;
	char *p_end = (char*)input + length;
	int  loop_count = 0;

	// 0x30 -> 00110000  
	// 0x3C -> 00111100  
	// 0x3F -> 00111111  
	while (p_end - p >= 3) {
		*p_dst++ = base64_index[(p[0] >> 2)];
		*p_dst++ = base64_index[((p[0] << 4) & 0x30) | (p[1] >> 4)];
		*p_dst++ = base64_index[((p[1] << 2) & 0x3C) | (p[2] >> 6)];
		*p_dst++ = base64_index[p[2] & 0x3F];
		p += 3;
	}

	if (p_end - p > 0) {
		*p_dst++ = base64_index[(p[0] >> 2)];
		if (p_end - p == 2) {
			*p_dst++ = base64_index[((p[0] << 4) & 0x30) | (p[1] >> 4)];
			*p_dst++ = base64_index[(p[1] << 2) & 0x3C];
			*p_dst++ = '=';
		}
		else if (p_end - p == 1) {
			*p_dst++ = base64_index[(p[0] << 4) & 0x30];
			*p_dst++ = '=';
			*p_dst++ = '=';
		}
	}

	*p_dst = '\0';
	return output;
}

char *base64_decode(const char *input, char *output)
{

	output[0] = '\0';
	if (input == NULL || output == NULL)
		return output;


	int input_len = strlen(input);
	if (input_len < 4 || input_len % 4 != 0)
		return output;

	// 0xFC -> 11111100  
	// 0x03 -> 00000011  
	// 0xF0 -> 11110000  
	// 0x0F -> 00001111  
	// 0xC0 -> 11000000  
	char *p = (char*)input;
	char *p_out = output;
	char *p_end = (char*)input + input_len;
	for (; p < p_end; p += 4) {
		*(p_out++) = ((get_index(p[0]) << 2) & 0xFC) | ((get_index(p[1]) >> 4) & 0x03);
		*(p_out++) = ((get_index(p[1]) << 4) & 0xF0) | ((get_index(p[2]) >> 2) & 0x0F);
		*(p_out++) = ((get_index(p[2]) << 6) & 0xC0) | (get_index(p[3]));

	}

	if (*(input + input_len - 2) == '=') {
		*(p_out - 2) = '\0';
	}
	else if (*(input + input_len - 1) == '=') {
		*(p_out - 1) = '\0';
	}
	return output;
}
