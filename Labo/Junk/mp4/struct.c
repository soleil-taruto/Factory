#include "C:\Factory\Common\all.h"

#define ROOT_NAME "root"

typedef struct Box_st
{
	char Name[5];
	autoBlock_t *Image;
	autoList_t *SubBoxes;
}
Box_t;

static Box_t *Root;

// ---- load ----

static autoList_t *LoadByImage(autoBlock_t *image)
{
	autoList_t *ret = newList();
	uint rPos = 0;
	uint broken = 0;

	while (rPos < getSize(image))
	{
		Box_t *i;
		uint size;
		uint name;

		if (getSize(image) - rPos < 8)
		{
//			LOGPOS();
			broken = 1;
			break;
		}
		size = *(uint *)(b_(image) + rPos + 0);
		name = *(uint *)(b_(image) + rPos + 4);

		size = revEndian(size);

		if (size < 8)
		{
//			LOGPOS();
			broken = 1;
			break;
		}
		rPos += 8;
		size -= 8;

		if (getSize(image) - rPos < size)
		{
//			LOGPOS();
			broken = 1;
			break;
		}
		i = nb_(Box_t);
		*(uint *)i->Name = name;
		i->Image = ab_makeSubBytes(image, rPos, size);
		i->SubBoxes = LoadByImage(i->Image);

		rPos += size;

		line2JLine(i->Name, 1, 0, 0, 1);

		addElement(ret, (uint)i);
	}
	if (broken)
	{
//		LOGPOS();
		setCount(ret, 0);
	}
	return ret;
}
static void LoadByFile(char *file)
{
	autoBlock_t *image = readBinary(file);
	autoList_t *boxes;

	boxes = LoadByImage(image);

	Root = nb_(Box_t);
	strcpy(Root->Name, ROOT_NAME);
	Root->Image = image;
	Root->SubBoxes = LoadByImage(image);

//	releaseAutoBlock(image);
	releaseAutoList(boxes);
}

// ---- show ----

static void ShowBox(Box_t *root, uint indent)
{
	Box_t *box;
	uint index;

	for (index = 0; index < indent; index++)
		cout("\t");

	cout("[%s] %u\n", root->Name, getSize(root->Image));

	foreach (root->SubBoxes, box, index)
		ShowBox(box, indent + 1);
}

// ---- find ----

static Box_t *FindBox(Box_t *root, char *name)
{
	Box_t *box;
	uint index;

	foreach (root->SubBoxes, box, index)
		if (!strcmp(box->Name, name))
			return box;

	return box;
}
static Box_t *GetBox(Box_t *root, char *name)
{
	Box_t *box = FindBox(root, name);

	errorCase_m(!box, xcout("NOT FOUND [%s]", name));
	return box;
}

// ---- to dir ---

static void ToDir_2(autoList_t *boxes)
{
	Box_t *box;
	uint index;

	foreach (boxes, box, index)
	{
		char *wDir = box->Name;

		wDir = lineToFairLocalPath(wDir, 100); // ‚±‚±‚Å•¡»I
		wDir = toCreatablePath(wDir, 1000);

		createDir(wDir);

		addCwd(wDir);
		{
			writeBinary("image.txt", box->Image);
			ToDir_2(box->SubBoxes);
		}
		unaddCwd();

		memFree(wDir);
	}
}
static void ToDir(char *wDir)
{
	removeDirIfExist(wDir);
	createDir(wDir);

	addCwd(wDir);
	{
		ToDir_2(Root->SubBoxes);
	}
	unaddCwd();
}

// ----

/*
static void DoTest01(void)
{
	LoadByFile("C:\\temp\\mp4\\ddd.mp4");
	ShowBox(Root, 0);
	ToDir("C:\\temp\\ddd");
}
*/
static void DoFile(char *file)
{
	LoadByFile(file);
	ShowBox(Root, 0);
	ToDir("C:\\temp\\mp4_root");
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		DoFile(nextArg());
		return;
	}

	{
		DoFile(c_dropFile());
	}
}
