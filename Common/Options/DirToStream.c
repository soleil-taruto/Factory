#include "DirToStream.h"

#define WRITER_BUFFSIZE (1024 * 1024 * 17)
#define READER_BUFFSIZE (1024 * 1024 * 16)

#define SIGN_DIR 'D'
#define SIGN_FILE 'F'
#define SIGN_INFO 'I'
#define SIGN_ENDDIR 0x00

void VTreeToStream(VTree_t *vt, void (*streamWriter)(uchar *, uint))
{
	uint count = vt->GetLocalCount();
	uint index;
	uchar buffer[9];

	for (index = 0; index < count; index++)
	{
		char *file = vt->GetLocal(index);

		streamWriter(file, strlen(file) + 1);

		if (vt->IsDir(index))
		{
			buffer[0] = SIGN_DIR;
			streamWriter(buffer, 1);

			vt->IntoDir(index);
			VTreeToStream(vt, streamWriter);
			vt->ExitDir();
		}
		else
		{
			uint64 size = vt->GetSize(index);
			uint64 readPos;
			uint readSize;
			void *block = memAlloc(WRITER_BUFFSIZE);

			buffer[0] = SIGN_FILE;
			value64ToBlock(buffer + 1, size);
			streamWriter(buffer, 9);

			for (readPos = 0; readPos < size; readPos += readSize)
			{
				readSize = m_min(WRITER_BUFFSIZE, (uint)(size - readPos));
				vt->GetEntity(index, readPos, readSize, block);
				streamWriter(block, readSize);
			}
			memFree(block);
		}
		memFree(file);
	}
	buffer[0] = SIGN_ENDDIR;
	streamWriter(buffer, 1);
}

int DTS_WithInfo;
int (*DTS_AcceptPath)(char *);

void DirToStream(char *dir, void (*streamWriter)(uchar *, uint))
{
	autoList_t *pathsStack = newList();
	autoList_t *infosStack = newList(); // !withInfo のときは使わない。
	autoList_t *paths;
	autoList_t *infos = NULL;
	int withInfo = DTS_WithInfo;

enterDir:
	addCwd(dir);

	if (withInfo)
	{
		lsInfos = newList();
		paths = ls(".");
		infos = lsInfos;
		lsInfos = NULL;
	}
	else
	{
		paths = ls(".");
	}
	eraseParents(paths);

	if (withInfo)
	{
		char *path;
		uint index;

		foreach (paths, path, index)
		{
			path = strrm(path, 4);
			setElement(paths, index, (uint)path);
			valueToBlock(strchr(path, '\0') + 1, getElement(infos, index));
		}
		rapidSortLines(paths);

		foreach (paths, path, index)
		{
			setElement(infos, index, blockToValue(strchr(path, '\0') + 1));
		}
	}
	else
	{
		rapidSortLines(paths);
	}

	for (; ; )
	{
		uchar buffer[26];
//		uchar buffer[9]; // old

		while (getCount(paths))
		{
			char *path = (char *)unaddElement(paths);
			FILE *fp;
			autoBlock_t *block;

			if (DTS_AcceptPath)
			{
				char *absPath = makeFullPath(path);

				if (!DTS_AcceptPath(absPath))
				{
					if (withInfo)
						memFree((void *)unaddElement(infos));

					memFree(path);
					memFree(absPath);
					continue;
				}
				memFree(absPath);
			}
			streamWriter(path, strlen(path) + 1);

			if (withInfo)
			{
				lsInfo_t *info = (lsInfo_t *)unaddElement(infos);

				buffer[0] = SIGN_INFO;
				buffer[1] =
					info->attrArch     << 3 |
					info->attrHidden   << 2 |
					info->attrReadOnly << 1 |
					info->attrSystem;
				value64ToBlock(buffer +  2, getFileStampByTime(info->createTime));
				value64ToBlock(buffer + 10, getFileStampByTime(info->accessTime));
				value64ToBlock(buffer + 18, getFileStampByTime(info->writeTime));

				memFree(info);
				streamWriter(buffer, 26);
			}
			if (existDir(path))
			{
				buffer[0] = SIGN_DIR;
				streamWriter(buffer, 1);

				dir = path;
				addElement(pathsStack, (uint)paths);

				if (withInfo)
					addElement(infosStack, (uint)infos);

				goto enterDir;
			}
			buffer[0] = SIGN_FILE;
			value64ToBlock(buffer + 1, getFileSize(path));
			streamWriter(buffer, 9);

			fp = fileOpen(path, "rb");

			while (block = readBinaryStream(fp, WRITER_BUFFSIZE))
			{
				streamWriter(directGetBuffer(block), getSize(block));
				releaseAutoBlock(block);
			}
			fileClose(fp);

			memFree(path);
		}
		releaseAutoList(paths);

		buffer[0] = SIGN_ENDDIR;
		streamWriter(buffer, 1);

		if (!getCount(pathsStack))
		{
			break;
		}
		paths = (autoList_t *)unaddElement(pathsStack);

		if (withInfo)
			infos = (autoList_t *)unaddElement(infosStack);

		unaddCwd();
	}
	releaseAutoList(pathsStack);
	releaseAutoList(infosStack);
	unaddCwd();
}

int STD_TrustMode;
int STD_ReadStop;

static void (*STD_StreamReader)(uchar *, uint);

static void STD_ReadStream(uchar *block, uint size)
{
	if (!STD_ReadStop)
	{
		STD_StreamReader(block, size);

		if (!STD_ReadStop)
		{
			return;
		}
	}
	memset(block, 0x00, size);
}
static time_t AdjustCAWTime(time_t t)
{
	uint mil;

	m_range(t, 19700102000000000i64, 30000101000000000i64); // 1970/1/2 ～ 3000/1/1

	mil = t % 1000;
	t /= 1000;
	t = compactStampToTime_x(xcout("%I64d", t));
	t = toValue64_x(makeCompactStamp(getStampDataTime(t))); // to fair YMDhms
	t *= 1000;
	t += mil;
	return t;
}
static time_t CheckAndAdjustCAWTime(time_t t, char *uiKind)
{
	time_t ta = AdjustCAWTime(t);

	if (t != ta)
	{
		cout("##################################\n");
		cout("## タイムスタンプを矯正しました ##\n");
		cout("##################################\n");
		cout("%s\n", uiKind);
		cout("< %I64u\n", t);
		cout("> %I64u\n", ta);

		t = ta;
	}
	return t;
}

/*
	dir - 出力先、存在する空のディレクトリであること。

	streamReader(uchar *block, uint size)
		次の size バイトを block に書き込む。
		要求されたサイズのデータは必ず返さなければならない。
		ユーザーから中断の要求があったり、これ以上データを用意できなくなるなど、中止したい場合は STD_ReadStop に !0 をセットする。
*/
void StreamToDir(char *dir, void (*streamReader)(uchar *, uint))
{
	uint dirDepth = 0;
	lsInfo_t info;
	int infoRdy = 0;

	STD_ReadStop = 0;
	STD_StreamReader = streamReader;

	addCwd(dir);

	for (; ; )
	{
		char *path = strx("");
		uchar buffer[8];

		for (; ; )
		{
			STD_ReadStream(buffer, 1);

			if (buffer[0] == '\0')
				break;

			path = addChar(path, buffer[0]);
		}

		if (path[0] == '\0') // ? == SIGN_ENDDIR
		{
			unaddCwd();

			if (!dirDepth)
			{
				memFree(path);
				break;
			}
			dirDepth--;
		}
		else
		{
			if (STD_TrustMode)
			{
				/*
					STD_TrustMode != 0 のときはどのようなパス名も許可されるが、
					フルパスや相対パスも許可されるため、システムフォルダ内のファイルを上書きすることも可能になる。
					流石にそれは怖いので、せめてそれらは弾く。
				*/
				errorCase(strchr(path, ':'));
				errorCase(mbs_strchr(path, '\\'));
			}
			else
			{
#if 1
				char *newPath = lineToFairLocalPath(path, strlen_x(getCwd()));

				if (strcmp(newPath, path))
				{
					line2JLine(path, 1, 0, 0, 1);

					cout("################################\n");
					cout("## ローカル名は矯正されました ##\n");
					cout("################################\n");
					cout("< %s\n", path);
					cout("> %s\n", newPath);
				}
				memFree(path);
				path = newPath;
#else // OLD CODE
				path = lineToFairLocalPath_x(path, strlen_x(getCwd()));
#endif

				errorCase(PATH_SIZE < strlen_x(getCwd()) + strlen(path));
			}
			STD_ReadStream(buffer, 1);

			if (buffer[0] == SIGN_INFO)
			{
				int af;

				STD_ReadStream(buffer, 1);
				af = buffer[0];

				info.attrArch     = m_01(af & 8);
				info.attrHidden   = m_01(af & 4);
				info.attrReadOnly = m_01(af & 2);
				info.attrSystem   = m_01(af & 1);

				STD_ReadStream(buffer, 8);
				info.createTime = blockToValue64(buffer);

				STD_ReadStream(buffer, 8);
				info.accessTime = blockToValue64(buffer);

				STD_ReadStream(buffer, 8);
				info.writeTime = blockToValue64(buffer);

				if (!STD_TrustMode)
				{
					info.createTime = CheckAndAdjustCAWTime(info.createTime, "Create");
					info.accessTime = CheckAndAdjustCAWTime(info.accessTime, "Access");
					info.writeTime  = CheckAndAdjustCAWTime(info.writeTime, "Write");
				}
				infoRdy = 1;
				STD_ReadStream(buffer, 1); // 再読込
			}
			if (buffer[0] == SIGN_DIR)
			{
				createDir(path);

				if (infoRdy)
				{
					setFileAttr(
						path,
						info.attrArch,
						info.attrHidden,
						info.attrReadOnly,
						info.attrSystem
						);

					// 日時はセットしない。

					infoRdy = 0;
				}
				addCwd(path);
				dirDepth++;
			}
			else if (buffer[0] == SIGN_FILE)
			{
				uint64 count;
				FILE *fp;

				STD_ReadStream(buffer, 8);
				count = blockToValue64(buffer);

				fp = fileOpen(path, "wb");

				while (0i64 < count && !STD_ReadStop)
				{
					autoBlock_t *block = nobCreateBlock(count < (uint64)READER_BUFFSIZE ? (uint)count : READER_BUFFSIZE);

					STD_ReadStream(directGetBuffer(block), getSize(block));
					writeBinaryBlock(fp, block);
					count -= getSize(block);

					releaseAutoBlock(block);
				}
				fileClose(fp);

				if (infoRdy)
				{
					// 書き込み禁止にすると日時を設定できなくなるので、先に！
					setFileStamp(
						path,
						info.createTime,
						info.accessTime,
						info.writeTime
						);

					setFileAttr(
						path,
						info.attrArch,
						info.attrHidden,
						info.attrReadOnly,
						info.attrSystem
						);

					infoRdy = 0;
				}
			}
			else
			{
				error();
			}
		}
		memFree(path);
	}
}
