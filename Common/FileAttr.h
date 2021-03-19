void setFileAttr(char *path, int attrArch, int attrHidden, int attrReadOnly, int attrSystem);
int getFileAttr(char *path, uint mask);
int getFileAttr_Arch(char *path);
int getFileAttr_Hidden(char *path);
int getFileAttr_ReadOnly(char *path);
int getFileAttr_System(char *path);
