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
#define BMP_Header_Length 54  //图像数据在内存块中的偏移量

#pragma comment(lib, "ws2_32.lib")

//旋转角度
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

//函数power_of_two用于判断一个整数是不是2的整数次幂  
int power_of_two(int n)
{
	if (n <= 0)
		return 0;
	return (n & (n - 1)) == 0;
}

/* 函数load_texture
* 读取一个BMP文件作为纹理
* 如果失败，返回0，如果成功，返回纹理编号
*/
GLuint load_texture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;

	// 打开文件，如果失败，返回  
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
		return 0;

	// 读取文件中图象的宽度和高度  
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// 计算每行像素所占字节数，并根据此数据计算总像素字节数  
	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	// 根据总像素字节数分配内存  
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	// 读取像素数据  
	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放  
	// 若图像宽高超过了OpenGL规定的最大值，也缩放  
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max)
		{
			const GLint new_width = 256;
			const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形  
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			// 计算每行需要的字节数和总字节数  
			new_line_bytes = new_width * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;

			// 分配内存  
			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0)
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}

			// 进行像素缩放  
			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			// 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height  
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	// 分配一个新的纹理编号  
	glGenTextures(1, &texture_ID);
	if (texture_ID == 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// 绑定新的纹理，载入纹理并设置纹理参数  
	// 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复  
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
	glBindTexture(GL_TEXTURE_2D, lastTextureID);  //恢复之前的纹理绑定  
	free(pixels);
	return texture_ID;
}

//拆分字符串
vector<string> split(const string &s, const string &seperator){
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	while (i != s.size()){
		//找到字符串中首个不等于分隔符的字母；
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

		//找到又一个分隔符，将两个分隔符之间的字符串取出；
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
	//清除颜色缓存和深度缓存
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -7.0f);
	glRotatef(angle1, 1.0f, 0.0f, 0.0f);
	glRotatef(angle2, 0.0f, 0.0f, 1.0f);
	//glRotatef(rquad3, 0.0f, 1.0f, 0.0f);

	

	//手机底面 橙色
	glBindTexture(GL_TEXTURE_2D, texBottom);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.79f, 0.05f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, 1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, 1.3f);
	glEnd();

	
	//手机顶面 粉色
	glBindTexture(GL_TEXTURE_2D, texTop);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.69f, 0.79f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, 0.0325f, -1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, -1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, -1.3f);
	glEnd();

	
	//手机左侧面 紫色
	glBindTexture(GL_TEXTURE_2D, texLeft);
	glBegin(GL_QUADS);
	glColor3f(0.64f, 0.28f, 0.64f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.65f, -0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, 0.0325f, -1.3f);
	glEnd();

	//手机右侧面 黄色
	glBindTexture(GL_TEXTURE_2D, texRight);
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.95f, 0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, -0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.65f, 0.0325f, -1.3f);
	glEnd();

	//手机正面 蓝色
	glBindTexture(GL_TEXTURE_2D, texPhone);
	glBegin(GL_QUADS);
	glColor3f(0.6f, 0.85f, 0.92f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.65f, 0.0325f, -1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(0.65f, 0.0325f, 1.3f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.65f, 0.0325f, -1.3f);
	glEnd();

	//手机背面 绿色
	glBindTexture(GL_TEXTURE_2D, texBack);
	glBegin(GL_QUADS);
	glColor3f(0.71f, 0.9f, 0.11f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.65f, -0.0325f, -1.3f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.65f, -0.0325f, 1.3f);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.65f, -0.0325f, 1.3f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.65f, -0.0325f, -1.3f);
	glEnd();

	//绘制框线
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
	//交换双缓存
	glutSwapBuffers();

	ret = recv(sClient, szMessage, MSGSIZE, 0);
	szMessage[ret] = '\0';
	string data = szMessage;
	tem = split(data, " ");
	if (tem.size() != 0)
	{
		//定义为侧视图
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

//获取本机所有IP
void getIPs()
{
	WORD v = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(v, &wsaData); // 加载套接字库    

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
	cout << "第一步，输入手机端指定的端口号：" << endl;
	cin >> port;
	cout << "\n第二步，在手机端输入本机无线网卡IP：" << endl;
	getIPs();
	cout << "\n第三步，在手机端点击连接..." << endl;
	cout << "\n********等待连接********\n";

	//启动Socket服务
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

	//OpenGL配置
	glutInit(&argc, argv);
	//使用双缓存模式和深度缓存
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 350);
	glutInitWindowPosition(200, 200);
	glutCreateWindow("手机姿态");
	glEnable(GL_TEXTURE_2D);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	//激活深度测试，以隐藏被遮挡面
	glEnable(GL_DEPTH_TEST);


	//装载贴图
	texPhone = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\zhengmian.bmp");
	texBack = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\back.bmp");
	texTop = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\top.bmp");
	texBottom = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\bottom.bmp");
	texLeft = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\left.bmp");
	texRight = load_texture("F:\\PhonePresentation\\PhonePresentation\\PC\\PresentationServer\\res\\right.bmp");

	
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	//设置空闲时用的函数
	glutIdleFunc(display);
	glutMainLoop();

	return 0;
}