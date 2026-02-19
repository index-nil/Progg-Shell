#ifndef NEO_PAD_H
#define NEO_PAD_H

void editorInit();
void editorFree();
void editorOpen(const char *filename);
void editorSave();
void editorClose();
void editorProcessKeypress();
#endif // NEO_PAD_H