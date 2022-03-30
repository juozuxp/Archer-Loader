#pragma once

class SHA256
{
public:
	static void Hash(const void* Data, unsigned long long DataSize, void* Result);

private:
	static void sha256_init(struct _SHA256_CTX* ctx);
	static void sha256_update(struct _SHA256_CTX* ctx, const unsigned char* data, unsigned long long len);
	static void sha256_final(struct _SHA256_CTX* ctx, unsigned char* hash);
	static void sha256_transform(struct _SHA256_CTX* ctx, const unsigned char* data);
};
