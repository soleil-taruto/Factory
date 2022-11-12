#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\DirToStream.h"

static autoBlock_t *Cluster;

static void BlockToCluster(uchar *block, uint size)
{
	ab_addBlock(Cluster, block, size);
}
static void ClusterToBlock(uchar *block, uint size)
{
	unaddBytesRevToBlock(Cluster, block, size);
}

static void Test_DirToStream(char *dir)
{
	char *outDir = makeFreeDir();

	Cluster = newBlock();

	cout("DirToStream START\n");
	DirToStream(dir, BlockToCluster);
	cout("DirToStream END\n");

	reverseBytes(Cluster); // �ǂݍ��݂̂��ߋt�]

	cout("StreamToDir START\n");
	StreamToDir(outDir, ClusterToBlock);
	cout("StreamToDir END\n");

	errorCase(getSize(Cluster)); // ? �ǂݏI����Ă��Ȃ��B
	errorCase(!isSameDir(dir, outDir, 0));

	recurRemoveDir(outDir);
	memFree(outDir);
	releaseAutoBlock(Cluster);

	cout("OK\n");
}
int main(int argc, char **argv)
{
	for (; ; )
	{
		Test_DirToStream(dropDir());
	}
}
