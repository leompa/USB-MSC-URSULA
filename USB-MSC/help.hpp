//lecturas de linea
int ReadLine(File* file, char* str, size_t size) {
  uint8_t ch;
  int rtn;
  size_t n = 0;
  while (true) {
    // check for EOF
    if (!file->available()) {
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1) {
      // read error
      rtn = -1;
      break;
    }
    // Delete CR and Space.
    if (ch == '\r' || ch == ' ') {
      continue;
    }
    if (ch == '\n') {
      rtn = 0;
      break;
    }
    if ((n + 1) >= size) {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';
  return rtn;
}

//separa str
void DivideStr(char*str, char*s1, char*s2, char sym)
{
  int i, r, n1, n2;
  i = -1; n1 = 0; n2 = 0; r = 0;
  while (str[i + 1] != 0)
  {
    i++;
    if (str[i] == sym) {
      r++;
      continue;
    }
    if (str[i] == ' ') {
      continue;
    }
    if (r == 0) {
      s1[n1] = str[i];
      n1++;
      continue;
    }
    if (r == 1) {
      s2[n2] = str[i];
      n2++;
      continue;
    }
  }
  s1[n1] = 0; s2[n2] = 0;
}

String generateBoundary() {
  String boundary = "----";
  for (int i = 0; i < 16; i++) {
    char randomChar = random(33, 126); // ASCII printable characters range
    boundary += randomChar;
  }
  return boundary;
}
//=====================================================
