#include <gl\glut.h>
#include <gl\GLU.h>
#include <gl\GL.h>
#include <WINSOCK2.H>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>

using namespace std;

#define MSGSIZE 512
#define BMP_Header_Length 54  //ͼ���������ڴ���е�ƫ����

#pragma comment(lib, "ws2_32.lib")

//��ת�Ƕ�
GLfloat angle1 = 0;
GLfloat angle2 = 0;
GLfloat angle3 = 0;

GLuint texPhone;
GLuint texBack;
GLuint texTop;
GLuint texBottom;
GLuint texLeft;
GLuint texRight;

WSADATA wsaData;
SOCKET sListen;
SOCKET sClient;
SOCKADDR_IN local;
SOCKADDR_IN client;
char szMessage[MSGSIZE];
int ret;
int iaddrSize = sizeof(SOCKADDR_IN);
int port = 2017;
vector<string> tem;
double a;
double b;
double c;

//����power_of_two�����ж�һ�������ǲ���2����������  
int power_of_two(int n)
{
	if (n <= 0)
		return 0;
	return (n & (n - 1)) == 0;
}

/* ����load_texture
* ��ȡһ��BMP�ļ���Ϊ����
* ���ʧ�ܣ�����0������ɹ�������������
*/
GLuint load_texture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;

	// ���ļ������ʧ�ܣ�����  
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	// ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�  
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���  
	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	// �����������ֽ��������ڴ�  
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	// ��ȡ��������  
	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// �Ծ;ɰ汾�ļ��ݣ����ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������  
	// ��ͼ���߳�����OpenGL�涨�����ֵ��Ҳ����  
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max)
		{
			const GLint new_width = 256;
			const GLint new_height = 256; // �涨���ź��µĴ�СΪ�߳���������  
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			// ����ÿ����Ҫ���ֽ��������ֽ���  
			new_line_bytes = new_width * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;

			// �����ڴ�  
			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0)
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}

			// ������������  
			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			// �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height  
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	// ����һ���µ�������  
	glGenTextures(1, &texture_ID);
	if (texture_ID == 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// ���µ������������������������  
	// �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�  
	GLint lastTextureID = last_texture_ID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, lastTextureID);  //�ָ�֮ǰ�������  
	free(pixels);
	return texture_ID;
}

//����ַ���
vector<string> split(const string &s, const string &seperator){
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()){
		//�ҵ��ַ������׸������ڷָ�������ĸ��
		int flag = 0;
		while (i != s.size() && flag == 0){
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
			if (s[i] == seperator[x]){
				++i;
				flag = 0;
				break;
			}
		}

		//�ҵ���һ���ָ������������ָ���֮����ַ���ȡ����
		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0){
			for (string_size x = 0; x < seperator.size(); ++x)
			if (s[j] == seperator[x]){
				flag = 1;
				break;
			}
			if (flag == 0)
				++j;
		}
		if (i != j){
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

void display(void)
{
	//�����ɫ�������Ȼ���
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -7.0f);
	glRotatef(angle1, 1.0f, 0.0f, 0.0f);
	glRotatef(angle2, 0.0f, 0.0f, 1.0f);
	//glRotatef(rquad3, 0.0f, 1.0f, 0.0f);

	

	//�ֻ����� ��ɫ
	glBindTexture(GL_TEXTURE_2D, texBottom);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.79f, 0.05f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, 1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, 1.3f);
	glEnd();

	
	//�ֻ����� ��ɫ
	glBindTexture(GL_TEXTURE_2D, texTop);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.69f, 0.79f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, 0.0325f, -1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, -1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, -1.3f);
	glEnd();

	
	//�ֻ������ ��ɫ
	glBindTexture(GL_TEXTURE_2D, texLeft);
	glBegin(GL_QUADS);
	glColor3f(0.64f, 0.28f, 0.64f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.65f, -0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, 0.0325f, -1.3f);
	glEnd();

	//�ֻ��Ҳ��� ��ɫ
	glBindTexture(GL_TEXTURE_2D, texRight);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.95f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, -0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.65f, 0.0325f, -1.3f);
	glEnd();

	//�ֻ����� ��ɫ
	glBindTexture(GL_TEXTURE_2D, texPhone);
	glBegin(GL_QUADS);
	glColor3f(0.6f, 0.85f, 0.92f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.65f, 0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.65f, 0.0325f, -1.3f);
	glEnd();

	//�ֻ����� ��ɫ
	glBindTexture(GL_TEXTURE_2D, texBack);
	glBegin(GL_QUADS);
	glColor3f(0.71f, 0.9f, 0.11f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, -1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, -0.0325f, 1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, -0.0325f, 1.3f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, -1.3f);
	glEnd();

	//���ƿ���
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	//---1---
	glVertex3f(1.5, 1.5, 1.5);
	glVertex3f(-1.5, 1.5, 1.5);
	glVertex3f(-1.5, 1.5, 1.5);
	glVertex3f(-1.5, -1.5, 1.5);
	glVertex3f(-1.5, -1.5, 1.5);
	glVertex3f(1.5, -1.5, 1.5);
	glVertex3f(1.5, -1.5, 1.5);
	glVertex3f(1.5, 1.5, 1.5);
	//---2---
	glVertex3f(1.5, 1.5, -1.5);
	glVertex3f(-1.5, 1.5, -1.5);
	glVertex3f(-1.5, 1.5, -1.5);
	glVertex3f(-1.5, -1.5, -1.5);
	glVertex3f(-1.5, -1.5, -1.5);
	glVertex3f(1.5, -1.5, -1.5);
	glVertex3f(1.5, -1.5, -1.5);
	glVertex3f(1.5, 1.5, -1.5);
	//---3---
	glVertex3f(1.5, 1.5, 1.5);
	glVertex3f(1.5, -1.5, 1.5);
	glVertex3f(1.5, -1.5, 1.5);
	glVertex3f(1.5, -1.5, -1.5);
	glVertex3f(1.5, -1.5, -1.5);
	glVertex3f(1.5, 1.5, -1.5);
	glVertex3f(1.5, 1.5, -1.5);
	glVertex3f(1.5, 1.5, 1.5);
	//---4---
	glVertex3f(-1.5, 1.5, 1.5);
	glVertex3f(-1.5, -1.5, 1.5);
	glVertex3f(-1.5, -1.5, 1.5);
	glVertex3f(-1.5, -1.5, -1.5);
	glVertex3f(-1.5, -1.5, -1.5);
	glVertex3f(-1.5, 1.5, -1.5);
	glVertex3f(-1.5, 1.5, -1.5);
	glVertex3f(-1.5, 1.5, 1.5);
	glEnd();
	//����˫����
	glutSwapBuffers();

	ret = recv(sClient, szMessage, MSGSIZE, 0);
	szMessage[ret] = '\0';
	string data = szMessage;
	tem = split(data, " ");
	if (tem.size() != 0)
	{
		//����Ϊ����ͼ
		a = atof(tem[0].c_str());
		b = atof(tem[1].c_str());
		//c = atof(tem[2].c_str());
		cout << a << " " << b << endl;
		angle1 = a;
		angle2 = b;
		angle3 = 0;
	}
	else
	{
		angle1 = 0;
		angle2 = 0;
		angle3 = 0;
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		//exit(0);
		break;
	default:
		break;
	}
}

//��ȡ��������IP
void getIPs()
{
	WORD v = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(v, &wsaData); // �����׽��ֿ�    

	int i = 0;
	struct hostent *phostinfo = gethostbyname("");
	for (i = 0; NULL != phostinfo&& NULL != phostinfo->h_addr_list[i]; ++i)
	{
		char *pszAddr = inet_ntoa(*(struct in_addr *)phostinfo->h_addr_list[i]);
		printf("%s\n", pszAddr);
	}

	WSACleanup();
}

int main(int argc, char** argv)
{
	cout << "��һ���������ֻ���ָ���Ķ˿ںţ�" << endl;
	cin >> port;
	cout << "\n�ڶ��������ֻ������뱾����������IP��" << endl;
	getIPs();
	cout << "\n�����������ֻ��˵������..." << endl;
	cout << "\n********�ȴ�����********\n";

	//����Socket����
	WSAStartup(0x0202, &wsaData);

	sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sListen, (struct sockaddr *) &local, sizeof(SOCKADDR_IN));

	listen(sListen, 1);

	sClient = accept(sListen, (struct sockaddr *) &client, &iaddrSize);
	printf("Accepted client:%s:%d\n", inet_ntoa(client.sin_addr),
		ntohs(client.sin_port));

	//OpenGL����
	glutInit(&argc, argv);
	//ʹ��˫����ģʽ����Ȼ���
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 350);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("�ֻ���̬");
	glEnable(GL_TEXTURE_2D);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	//������Ȳ��ԣ������ر��ڵ���
	glEnable(GL_DEPTH_TEST);


	//װ����ͼ
	texPhone = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\zhengmian.bmp");
	texBack = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\back.bmp");
	texTop = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\top.bmp");
	texBottom = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\bottom.bmp");
	texLeft = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\left.bmp");
	texRight = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\right.bmp");

	
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	//���ÿ���ʱ�õĺ���
	glutIdleFunc(display);
	glutMainLoop();

	return 0;
}