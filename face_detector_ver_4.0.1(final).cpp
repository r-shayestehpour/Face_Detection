#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

using namespace std;

// ********************************************************** declaration of classes **************************************************************************

class pixel{

	double r, g; // this variabals are RGBnormalize variabales
	double h, s, l; // this variabals are RGBtoHSL variabales

	public:

	unsigned char red, green, blue; // the value of rgb of this pixel in the pictur
	bool skin; // if it was 1 it means that this pixel known as skin and if it was 0 this pixel doesn't show a skin
	int x, y; // it shows the position of the pixel in the matrix of image

	bool mark;// it's for bfs function in getting objects

	//filtering
	void RGBtoHSL(); // it sets the value of h, s and l 
	void RGBnormalize(); // it sets the value of r and g
	void set_skin(); // it sets the value of skin that if the color of this pixel is the color of body set the skin 1 and the other case set it 0

};

class object{

	int tool, arz, area; // tool = height of bounding box of object & arz = width of bounding box of object & area = area of bounding box of object
	int min_x, max_x, min_y, max_y; //min_x = minimum x that the members of object is cordinated in that min_y and max_x and max_y are declared in the same way 


	bool goodarea(int h, int w);//if the area of the object has a good ratio with the area of image (h*w)that it could be a face this function returns 1
	bool goodpercentage(); // if the percentage of the skin pixel in the bounding box of the object was good it could be a face and this function returns 1

	public:
	
	vector<pixel> member; // it save the pixel of the object


	void init(int h, int w);// it sets the valu of arz, tool, area, max_x, max_y, min_x, min_y by the members of object
	bool isface(int h, int w);// it check if the object is face or not by the goodarea and goodpercentage test
	bool goodnesbat(); // if the ratio arz/tool was in a good range the object could be a face and this function returns 1
	void print_border(FILE *f1, int height, int width);// it print a border in red color around the object 
	int check_overlap(); // it chekck that the object have two or more face or object in itself or not by checking that if existing a line that the number of skin pixels in it is low enough that could break the object from this line to two object and its return value is the andis of pixel in member vector that from this andis the object should break to two object!
};

class photo{
    
    public:

	FILE *f, *f_skin, *f_face;
	int width, height, pad;
	char address[100], address_end[100];// the address of picture

	vector <vector <pixel> > matrix; // it seves the pixels in the image
	vector <object>  all_object, face;


	//global functions
	void padding(FILE *f1);// it handels padding in reading or writing pixels
	bool isinmat(int y, int x); // it checks that the point (y, x) is in the range of matrix of pixels or not


	//reading and initialization
	bool headervalidation();// it use in read function and validate the header of the image
	void badinput();// it use in read function and if the input address is not match with the program handel it
	void read_size();// it read the width and the height of the image
	void init_matrix();//it initialize matrix of pixels
	void read_matrix();// it read the pixels of the image
	void resize_matrix();// it resize the matrix of pixels to width and height
		
	
	//detecting faces

	//filtering
	void find_skin();

	//printing
	void copy_header(FILE *f1); // it copy the header of the orginal image in f1
	void copy_matrix(FILE *f1); // it capy the matrix of pixels of the original image in f1
	void bordering(FILE *f1); // it porders around all faces
	void print_skin(); // it produces a white&black image with name of "face_skin.bmp" in the sorce code directory from the orginal image that if the pixel is skin it is white in this picture otherwise it seems black

	//getting objects
	void get_object();// it sets all the objects that are in the picture
	vector<pixel> bfs(pixel u);// find the component of the conecting pixel
	
	//handeling faces
	void find_face(); //it checks all_object. if an object have the good area (reletiv to the area of the image) and goodpercentage of skin pixel in its bounding box this funtion add this to the face vector
	void overlap_face(); // it check each face that is detect from find face function if it forms from two or more object or not.(if it hse a vertical line that have low number of skin pixel it means two object overlap to eachother) and then this function break this object from this vertical line to two object 
	void check_nesbat(); // after handeling overlaps, this function eliminate the faces that the ratio of arz and tool is not good in them
	


	public:
	void read(); // read the image and initialize the values
	void face_detect();// detect the faces in the picture
	void make_finalimage(); // it produce the result image 
	void closing(); // it close all the files that are open in the project
};



// *********************************************************declaration of functions **************************************************************************



//prototype of usual functions

void print_color(FILE *f1, unsigned char r, unsigned char g, unsigned char b);// it write in the file f1 a pixel with the color of rgb
bool cmp(pixel a, pixel b); // it compare two pixel first with height and then with width and it use for the sort function that use in check overlap function


// pixel's function *************************************************************************************************

		//finding skins
void pixel::RGBtoHSL(){

	double max1, min1;
	double r1 = (double)red / 255;
	double b1 = (double)blue / 255;
	double g1 = (double)green / 255; 

	max1 = max(r1, max(g1, b1));
	min1 = min(r1, min(g1, b1));

	l = (max1 + min1) / 2;
	if (max1 == min1){
		s = 0;
		h = 0;	
		l = (int)(l * 240);
		return;
	}
	else{
		if (l < 0.5){
			s = (max1 - min1) / (max1 + min1);
		}
		else{
			s = ( max1 - min1) / (2 - max1 - min1);
		}
		if(r1 == max1){
			h = (g1 - b1) / (max1 - min1);	
		}
		else if (g1 == max1){
			h = 2 + ((b1 - r1) / (max1 - min1));
		}
		else if(b1 == max1){
			h = 4 + ((r1 - g1) / (max1 - min1));
		}
	}
	if(h < 0)
		h += 6;
	h = (int) (h * 40);
	l *= (int) (h * 240);
	s *= (int) (h *240);
}



void pixel::RGBnormalize(){
	double sum = (int)red + green + blue;
	r = red / sum;
	g = green / sum;
}



void pixel::set_skin(){
	
	skin = 0;

	if(h >= 20 && h < 239)
		return;

	double f1,f2,w;	
	f1 = -1.376 * r * r + 1.0743 * r + 0.2;
	f2 = -0.776 * r * r + 0.5601 * r + 0.18;
	w = (r - 0.33) * ( r - 0.33) + (g - 0.33) * (g - 0.33);	
	if (g > f2 && g < f1 && w > 0.0015) 
		skin=1;
}





// object functions ***********************************************************************************


// initialization of object
void object::init(int h, int w){
	min_x = w - 1;
	min_y = h - 1;
	max_y = 0;
	max_x = 0;
	int s = member.size();
	for(int i = 0; i < s; i ++){
		if(member[i].x < min_x)
			min_x = member[i].x;
		if(member[i].y < min_y)
			min_y = member[i].y;
		if(member[i].x > max_x)
			max_x = member[i].x;
		if(member[i].y > max_y)
			max_y = member[i].y;
	}
	arz = max_x - min_x + 1;
	tool = max_y - min_y + 1;
	area = tool * arz;
}


// detecting face from other objects
bool object::isface(int h, int w){
	if(!goodarea(h, w))
		return false;
	if(!goodpercentage())
		return false;
	return true;
}

bool object::goodarea(int h, int w){
	if( area < h * w * 0.0035 || area > h * w * 0.2)
		return false;
	return true;
}

bool object::goodpercentage(){
	double percentage = 1.0 * member.size() / (tool * arz);
	if(percentage > 0.9 || percentage < 0.48)
		return false;
	return true;

}

bool object::goodnesbat(){
	double nesbat = 1.0 * tool / arz;
	if(nesbat > 2.2 || nesbat < 1)
		return false;
	return true;
}


int object::check_overlap(){
	
	sort(member.begin(), member.end(), cmp);

	int co = 0;
	for(int i = min_y; i <= max_y; i ++){
		int sum = 0;
		while(co < member.size() && member[co].y == i){
			sum ++;
			co ++;
		}
		double nesbat = 1.0 * sum / arz;
		if(nesbat < 0.2)
			return co;
	}
	return -1;
}



// printing border around this object
void object::print_border(FILE *f1, int height, int width){

	int pad = (4 - (width * 3) % 4) % 4;
	int num_seek = 54 + (3 * width + pad) * min_y + min_x * 3;

	fseek(f1, num_seek * sizeof(char), SEEK_SET);
	
	for(int i = 0; i < arz; i ++)
		print_color(f1, 255, 0, 0);
	
	for(int i = 0; i < tool - 2 ; i ++){
		num_seek += width * 3 + pad;
		fseek(f1, num_seek * sizeof(char), SEEK_SET);
		print_color(f1, 255, 0, 0);
		fseek(f1, (arz - 2) * 3 * sizeof(char), SEEK_CUR);
		print_color(f1, 255, 0, 0);
	}

	num_seek += width * 3 + pad;
	fseek(f1, num_seek * sizeof(char), SEEK_SET);
	for(int i = 0; i < arz; i ++)
		print_color(f1, 255, 0, 0);
	
}




//photo's functions ***********************************************************************************************************
		
		

		//global photo functions		
void photo::padding(FILE *f1){
	fseek(f1, pad * sizeof(char), SEEK_CUR);
}

bool photo::isinmat(int y, int x){
	if(x < 0 || x >= width || y < 0 || y >= height)
		return 0;
	return 1;
}
		


		// reading and initialization 
void photo::read(){
	cout << "enter your bitmap pictur address:";
	cin >> address;

	f = fopen(address, "rb");

	if(f == NULL || !headervalidation())
		badinput();	

	read_size();
	resize_matrix();
	init_matrix();
	read_matrix();
}


bool photo::headervalidation(){
	char headertype[2];
	fseek(f, 0, SEEK_SET);
	fread(&headertype[0], sizeof(char), 1, f);
	fread(&headertype[1], sizeof(char), 1, f);
	if(headertype[0] != 'B' || headertype[1] != 'M')
		return 0;
	return 1;
}

void photo::badinput(){
	cerr << "Your address is not valid! try again;" << endl; 
	read();
}

void photo::read_size(){
	fseek(f, 18*sizeof(char), SEEK_SET);
	fread(&width, sizeof(int), 1, f);
	fread(&height, sizeof(int), 1, f);
	pad = (4 - (3 * width) % 4) % 4;
}

void photo::init_matrix(){
	for(int i = 0; i < height; i ++)
		for(int j = 0; j < width; j ++){
			matrix[i][j].x = j;
			matrix[i][j].y = i;
			matrix[i][j].mark = false;
		}
}

void photo::resize_matrix(){
	matrix.resize(height);
	for(int i = 0; i < height ; i++)
		matrix[i].resize(width);
}


void photo::read_matrix(){
	fseek(f, 54 * sizeof(char), SEEK_SET);
	for(int i = 0; i < height; i ++){
		for(int j = 0; j < width; j ++){
			fread(&matrix[i][j].blue, sizeof(char), 1, f);
			fread(&matrix[i][j].green, sizeof(char), 1, f);
			fread(&matrix[i][j].red, sizeof(char), 1, f);
		}
		padding(f);
	}
}





		//face_detection
void photo::face_detect(){
	find_skin();
	get_object();
	find_face();
	overlap_face();
	check_nesbat();
	print_skin();
}


					// finding skins from the pixels
void photo::find_skin(){	
	for(int i=0;i<height;i++)
		for(int j=0;j<width;j++){
			matrix[i][j].RGBnormalize();
			matrix[i][j].RGBtoHSL();
			matrix[i][j].set_skin();
		}
}



					// printing and making images
void photo::print_skin(){
	f_skin = fopen("face_skin.bmp", "wb");
	copy_header(f_skin);
	for(int i = 0; i < height; i ++){
		for(int j = 0; j < width; j ++)
			if(matrix[i][j].skin)
				print_color(f_skin, 255, 255, 255);
			else
				print_color(f_skin, 0, 0, 0);
        char tmp = 0;
		fwrite(&tmp, sizeof(char), pad, f_skin);
	}
	bordering(f_skin);
}

void photo::copy_header(FILE *f1){
	unsigned char tmp[60];
	fseek(f, 0, SEEK_SET);
	fseek(f1, 0, SEEK_SET);
	fread(tmp, sizeof(unsigned char), 54, f);
	fwrite(tmp, sizeof(unsigned char), 54, f1);
}

void photo::copy_matrix(FILE *f1){
	for(int i = 0; i < height; i ++){
	   for(int j = 0; j < width; j ++)
	   		print_color(f1, matrix[i][j].red, matrix[i][j].green, matrix[i][j].blue); 
        char tmp = 0;
		fwrite(&tmp, sizeof(char), pad, f1);
	}
}

void photo::bordering(FILE *f1){
	int s = face.size();
	for(int i = 0; i < s; i ++)
		face[i].print_border(f1, height, width);
}



					// geting object
void photo::get_object(){
	for(int i = 0; i < height; i ++)
		for(int j = 0; j < width; j ++)
			if(!matrix[i][j].mark){
				matrix[i][j].mark = true;
				if(matrix[i][j].skin){
					object w;
					w.member = bfs(matrix[i][j]);
					w.init(height, width);
					all_object.push_back(w);
				}
			}
}

vector<pixel> photo::bfs(pixel u){
	vector<pixel> que;
	que.push_back(u);
	for(int i = 0; i < que.size(); i ++){	
		pixel tmp = que[i];
		for(int j = -1; j < 2; j ++)
			for(int k = -1; k < 2; k ++){
				int tmp_x = tmp.x + k;
				int tmp_y = tmp.y + j;
				if(isinmat(tmp_y , tmp_x)){
					if(!matrix[tmp_y][tmp_x].mark && matrix[tmp_y][tmp_x].skin){
						que.push_back(matrix[tmp_y][tmp_x]);
						matrix[tmp_y][tmp_x].mark = true;
					}
				}	
			}
	}
	return que;
}
				

					// finding faces from all_object 

void photo::find_face(){
	int s = all_object.size();
	for(int i = 0; i < s; i ++)
		if(all_object[i].isface(height, width))
			face.push_back(all_object[i]);
}

void photo::overlap_face(){
	for(int i = 0; i < face.size(); i ++){
		int tmp = face[i].check_overlap();
		if(tmp != -1){
			object w;
			w.member = face[i].member;
			w.member.erase(w.member.begin(), w.member.begin() + tmp);
			w.init(height, width);
			if(w.isface(height, width)){
				face.push_back(w);
			}

			face[i].member.erase(face[i].member.begin() + tmp, face[i].member.end());
			face[i].init(height, width);
			if(!face[i].isface(height, width) || !face[i].goodnesbat()){
				face.erase(face.begin() + i);
				i --;
			}
		}
	}
}

void photo::check_nesbat(){
	for(int i = 0; i < face.size(); i ++)
		if(!face[i].goodnesbat()){
			face.erase(face.begin() + i);
			i --;
		}
}



	// produsing result image
void photo::make_finalimage(){
	cout << "enter your distination address for saving the result image:"; 
	cin >> address_end;
	f_face = fopen(address_end, "wb");
	copy_header(f_face);
	copy_matrix(f_face);
	bordering(f_face);
}



	// closing all files
void photo::closing(){
	fclose(f);
	fclose(f_skin);
	fclose(f_face);
}


// usual functions ******************************************************************************************************

void print_color(FILE *f1, unsigned char r, unsigned char g, unsigned char b){
	fwrite(&b, sizeof(char), 1, f1);
	fwrite(&g, sizeof(char), 1, f1);
	fwrite(&r, sizeof(char), 1, f1);
}

bool cmp(pixel a, pixel b){
	if(a.y != b.y)
		return a.y < b.y;
	return a.x < b.x;
}



// ******************************************************************** main function *************************************************************************


int main(){

	photo aks;
	aks.read();
	aks.face_detect();
	aks.make_finalimage();
	aks.closing();

	return 0;
}
