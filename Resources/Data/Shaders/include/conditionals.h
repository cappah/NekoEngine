/*
 * Functions to avoid shader conditionals
 * http://theorangeduck.com/page/avoiding-shader-conditionals
 */

float when_eq(float x, float y) {
	return 1.0 - abs(sign(x - y));
}

float when_neq(float x, float y) {
	return abs(sign(x - y));
}

float when_gt(float x, float y) {
	return max(sign(x - y), 0.0);
}

float when_lt(float x, float y) {
	return max(sign(y - x), 0.0);
}

float when_ge(float x, float y) {
	return 1.0 - when_lt(x, y);
}

float when_le(float x, float y) {
	return 1.0 - when_gt(x, y);
}

float when_and(float a, float b) {
	return a * b;
}

float when_or(float a, float b) {
	return min(a + b, 1.0);
}

float when_not(float a) {
	return 1.0 - a;
}
