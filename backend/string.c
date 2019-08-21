UINT16 length(String s) {
  UINT16 i = 1;
  while(s[i++]);
  return --i;
}

bool streql(String ch1, String ch2) {
  UINT8 size = length(ch1);
	if(size != length(ch2)) return false;
	for(UINT8 i = 0; i <= size; i++)
		if(ch1[i] != ch2[i])
      return false;
	return true;
}


String *split(String str, CHAR16 ch) {
  UINT8 size = length(str);
  UINT8 arrsize = 0;
  String *arr = {L""};
  UINT8 tmpsize = 0;
  String tmp = L"";
	for(UINT8 i = 0; i <= size; i++)
		if(str[i] == ch) {
      tmp[tmpsize] = 0;
      arr[arrsize] = tmp;
      tmpsize = 0;
    } else {
      tmp[tmpsize] = str[i];
      tmpsize++;
    }
	return arr;
}

void append(String dest, const String src) {
  StrCat(dest, src);
}
