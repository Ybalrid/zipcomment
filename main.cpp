#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

static constexpr uint32_t ZIP_SIG = 0x06054b50;

#pragma pack(push, 1)
struct ZipEOCD
{
	uint32_t eocds;				// 0  4 must be == to ZIP_SIG
	uint16_t nbdisk;			// 4  2
	uint16_t dwithcd;			// 6  2
	uint16_t nbcdrec;			// 8  2
	uint16_t tnbdcrec;			// 10 2
	uint32_t sizecd;			// 12 4
	uint32_t startreltostart;	// 16 4
	uint16_t nbcharcom;			// 20 2 length of the comment string
	//char commentStr[];		// 22 ==nbcharcom bytes
};
#pragma pack(pop)

struct work
{
	const char* zipPath = nullptr;
	const char* zipComment = nullptr;
};

int execute(work w)
{
	FILE* zipFile = fopen(w.zipPath, "rb+");
	if(!zipFile)
	{
		fprintf(stderr, "Cannot open %s\n", w.zipPath);
		return EXIT_FAILURE;
	}

	fseek(zipFile, 0, SEEK_END);
	const auto size = ftell(zipFile);
	rewind(zipFile);

	bool found = false;
	long position = 0;
	for (long i = sizeof(ZIP_SIG); i <= (size); ++i)
	{
		position = size - (i);
		uint32_t signature = 0;
		fseek(zipFile, position, SEEK_SET);
		fread(&signature, sizeof(ZIP_SIG), 1, zipFile);
		if (0 == memcmp(&signature, &ZIP_SIG, sizeof(ZIP_SIG)))
		{
			found = true;
			break;
		}
	}

	if (found)
	{
		ZipEOCD eocd{};
		fseek(zipFile, position, SEEK_SET);
		fread(&eocd, sizeof(eocd), 1, zipFile);
		fseek(zipFile, position, SEEK_SET);
		eocd.nbcharcom = strlen(w.zipComment);
		fwrite(&eocd, sizeof(eocd), 1, zipFile); //overwrited EOCD
		//cursor is now back at the beginning of the comment string position, conveniently
		if(eocd.nbcharcom > 0)
			fwrite(w.zipComment, 1, strlen(w.zipComment), zipFile);
	}

	fclose(zipFile);

	if(!found)
	{
		fprintf(stderr, "Did not find the EOCD signature in the provided file. Are you sure this is a valid Zip archive ?\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int usage()
{
	std::cout << "USAGE:\n"
		<< "  -  zipcomment <path to zip file> \"Comment string here, can be empty but must be present\"\n";

	return EXIT_FAILURE;
}

void banner() { std::cout << "zipcomment (c) 2022 Arthur Brainville (Ybalrid)" << std::endl; }

int main(int argc, char** argv)
{
	banner();

	if (argc < 3)
		return usage();

	return execute({ argv[1], argv[2] });
}
