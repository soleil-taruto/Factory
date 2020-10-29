void *memCalloc(uint size);
void *memCloneWithBuff(void *block, uint size, uint buffsize);
void *memClone(void *block, uint size);

#define na_(TYPE_T, num) \
	((TYPE_T *)memCalloc(sizeof(TYPE_T) * (num)))

#define nb_(TYPE_T) \
	(na_(TYPE_T, 1))
