#include <stdio.h>
#include <math.h>

// fac = PI/180
#define FAC 0.01745329252

#define MIDDLE_X 29
#define MIDDLE_Y 31

#define LENGTH_H 10
#define LENGTH_M 13

int main(void) {

	float i;
	printf("int xHour[] ={");
	for (i=0; i<12; i+=1.0) {
		printf(
			"%.0f,",
			LENGTH_H * cos((FAC * (i * 30.0 - 90.0))) + MIDDLE_X
		);
	}
	printf(
		"%.0f}\n",
		LENGTH_H * cos(FAC * (12.0 * 30.0 - 90.0)) + MIDDLE_X
	);

	printf("int yHour[] ={");
	for (i=0; i<12; i+=1.0) {
		printf(
			"%.0f,",
			LENGTH_H * sin((FAC * (i * 30.0 - 90.0))) + MIDDLE_Y
		);
	}
	printf(
		"%.0f}\n",
		LENGTH_H * sin(FAC * (12.0 * 30.0 - 90.0)) + MIDDLE_Y
	);



	printf("int xMin[] ={");
	for (i=0; i<59; i+=1.0) {
		printf(
			"%.0f,",
			LENGTH_M * cos((FAC * (i * 6.0 - 90.0))) + MIDDLE_X
		);
	}
	printf(
		"%.0f}\n",
		LENGTH_M * cos(FAC * (59.0 * 6.0 - 90.0)) + MIDDLE_X
	);
	
	printf("int yMin[] ={");
	for (i=0; i<59; i+=1.0) {
		printf(
			"%.0f,",
			LENGTH_M * sin((FAC * (i * 6.0 - 90.0))) + MIDDLE_Y
		);
	}
	printf(
		"%.0f}\n",
		LENGTH_M * sin(FAC * (59.0 * 6.0 - 90.0)) + MIDDLE_Y
	);

	return 0;
}
