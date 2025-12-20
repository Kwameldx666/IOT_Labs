#include "CustomSTDIO.h"

static FILE* g_stdioStream = nullptr;

static int putChar(char ch, FILE* /*stream*/) {
	return Serial.write(ch);
}

static int getChar(FILE* /*stream*/) {
	while (!Serial.available()) {
	}
	return Serial.read();
}

void StdioSerialSetup() {
	if (g_stdioStream) {
		return;
	}
	Serial.begin(9600);
	while (!Serial) {
	}

	g_stdioStream = fdevopen(putChar, getChar);
	stdin = g_stdioStream;
	stdout = g_stdioStream;
}

void printFloat(const char* label, float value, const char* suffix) {
	char buf[24];
	dtostrf(value, 0, 2, buf);
	printf("%s%s%s\r\n", label, buf, suffix);
}

void stdioFlush() {
	fflush(stdout);
	Serial.flush();
}

bool stdioHasData() {
	return Serial.available() > 0;
}

String stdioGetString() {
	return Serial.readStringUntil('\n');
}
