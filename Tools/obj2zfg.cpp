/* NekoEngine
 *
 * obj2zfg.cpp
 * Author: Alexandru Naiman
 *
 * .OBJ to NekoEngine .ZFG converter
 * !!! OBSOLETE !!!
 * .ZFG format no longer used by the engine. Files in this format can be
 * be converted to .nmesh using the ModelConverter (macOS only)
 *
 * -----------------------------------------------------------------------------
 * 
 * Copyright (c) 2015-2016, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include <zlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define LINE_BUFF	1024

using namespace std;

struct GroupInfo
{
	size_t vertices;
	size_t indices;
};

struct VertexFormat{
		float position_x, position_y, position_z;
		float normal_x, normal_y, normal_z;
		float texcoord_x, texcoord_y;
		glm::vec3 tgt, binorm;
		VertexFormat(){
			position_x = position_y =position_z=0;
			normal_x = normal_y = normal_z=0;
			texcoord_x = texcoord_y=0;
		}
		VertexFormat(float px, float py, float pz ){
			position_x =px;		position_y =py;		position_z =pz;
			normal_x =normal_y= normal_z =0;
			texcoord_x=	texcoord_y=0;
		}
		VertexFormat(float px, float py, float pz, float nx, float ny, float nz){
			position_x =px;		position_y =py;		position_z =pz;
			normal_x =nx;		normal_y =ny;		normal_z =nz;
			texcoord_x=	texcoord_y=0;
		}
		VertexFormat(float px, float py, float pz, float tx, float ty){
			position_x =px;		position_y =py;		position_z =pz;
			texcoord_x=tx;		texcoord_y=ty;
			normal_x =normal_y= normal_z =0;
		}
		VertexFormat(float px, float py, float pz, float nx, float ny, float nz, float tx, float ty){
			position_x =px;		position_y =py;		position_z =pz;
			normal_x =nx;		normal_y =ny;		normal_z =nz;
			texcoord_x=tx;		texcoord_y=ty;
		}
		VertexFormat operator=(const VertexFormat &rhs){ 
			position_x = rhs.position_x;	position_y = rhs.position_y;	position_z = rhs.position_z;
			normal_x = rhs.normal_x;		normal_y = rhs.normal_y;		normal_z = rhs.normal_z;
			texcoord_x = rhs.texcoord_x;	texcoord_y = rhs.texcoord_y;
			return (*this);
		}
};

//helper funcs
float stringToFloat(const std::string &source){
	std::stringstream ss(source.c_str());
	float result;
	ss>>result;
	return result;
}

//transforms a string to an int
unsigned int stringToUint(const std::string &source){
	std::stringstream ss(source.c_str());
	unsigned int result;
	ss>>result;
	return result;
}

//transforms a string to an int
int stringToInt(const std::string &source){
	std::stringstream ss(source.c_str());
	int result;
	ss>>result;
	return result;
}

//writes the tokens of the source string into tokens
void stringTokenize(const std::string &source, std::vector<std::string> &tokens){
	tokens.clear();
	std::string aux=source;	
	for(unsigned int i=0;i<aux.size();i++) if(aux[i]=='\t' || aux[i]=='\n') aux[i]=' ';
	std::stringstream ss(aux,std::ios::in);
	while(ss.good()){
		std::string s;
		ss>>s;
		if(s.size()>0) tokens.push_back(s);
	}
}

/**
 * Split string
 */
static inline std::vector<std::string> SplitString(std::string str, char delim)
{
	std::vector<std::string> vec;
	std::stringstream ss(str);
	std::string tok;

	while (getline(ss, tok, delim))
		vec.push_back(tok);

	return vec;
}

/**
 * Read a Vector2 from string (x, y)
 */
static inline glm::vec2 ReadVector2(std::string line)
{
	std::vector<std::string> vec = SplitString(line, ',');

	if (vec.size() != 2)
		return glm::vec2(0.f, 0.f);

	return glm::vec2(std::stof(vec[0].c_str()), std::stof(vec[1].c_str()));
}

/**
 * Read a Vector3 from string (x, y, z)
 */
static inline glm::vec3 ReadVector3(std::string line)
{
	std::vector<std::string> vec = SplitString(line, ',');

	if (vec.size() != 3)
		return glm::vec3(0.f, 0.f, 0.f);

	return glm::vec3(std::stof(vec[0].c_str()), std::stof(vec[1].c_str()), std::stof(vec[2].c_str()));
}

/**
 * Read a vertex from a nfg file
 */
static inline VertexFormat ReadVertex(std::string line)
{
	std::vector<std::string> vec = SplitString(line, ';');
		
	VertexFormat vert;
	glm::vec3 v3;
	glm::vec2 v2;

	for (std::string str : vec)
	{
		size_t found;
		str[str.length() - 1] = '\0'; // discard last square brace
			
		if ((found = str.find("pos")) != std::string::npos)
		{
			v3 = ReadVector3(str.substr(found + 5));
		
			vert.position_x = v3.x;
			vert.position_y = v3.y;
			vert.position_z = v3.z;
		}
		else if ((found = str.find("binorm")) != std::string::npos)
			vert.binorm = ReadVector3(str.substr(found + 8));
		else if ((found = str.find("norm")) != std::string::npos)
		{
			v3 = ReadVector3(str.substr(found + 6));

			vert.normal_x = v3.x;
			vert.normal_y = v3.y;
			vert.normal_z = v3.z;
		}
		else if ((found = str.find("tgt")) != std::string::npos)
			vert.tgt = ReadVector3(str.substr(found + 5));
		else if ((found = str.find("uv")) != std::string::npos)
		{
			v2 = ReadVector2(str.substr(found + 4));

			vert.texcoord_x = v2.x;
			vert.texcoord_y = v2.y;
		}
	}

	return vert;
}

//variant for faces
void faceTokenize(const std::string &source, std::vector<std::string> &tokens){
	std::string aux=source;	
	for(unsigned int i=0;i<aux.size();i++) if(aux[i]=='\\' || aux[i]=='/') aux[i]=' ';
	stringTokenize(aux,tokens);		
}

void loadObjFile(const std::string &filename, std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices, std::vector<GroupInfo> &groups) {
	//citim din fisier
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
	if (!file.good()) {
		std::cout << "Mesh Loader: Nu am gasit fisierul obj " << filename << " sau nu am drepturile sa il deschid!" << std::endl;
		std::terminate();
	}

	std::string line;
	std::vector<std::string> tokens, facetokens;
	std::vector<glm::vec3> positions;		positions.reserve(1000);
	std::vector<glm::vec3> normals;		normals.reserve(1000);
	std::vector<glm::vec2> texcoords;		texcoords.reserve(1000);
	while (std::getline(file, line)) {
		//tokenizeaza linie citita
		stringTokenize(line, tokens);

		//daca nu am nimic merg mai departe
		if (tokens.size() == 0) continue;

		//daca am un comentariu merg mai departe
		if (tokens.size()>0 && tokens[0].at(0) == '#') continue;

		//daca am un vertex
		if (tokens.size()>3 && tokens[0] == "v") positions.push_back(glm::vec3(stringToFloat(tokens[1]), stringToFloat(tokens[2]), stringToFloat(tokens[3])));

		//daca am o normala
		if (tokens.size()>3 && tokens[0] == "vn") normals.push_back(glm::vec3(stringToFloat(tokens[1]), stringToFloat(tokens[2]), stringToFloat(tokens[3])));

		//daca am un texcoord
		if (tokens.size()>2 && tokens[0] == "vt") texcoords.push_back(glm::vec2(stringToFloat(tokens[1]), stringToFloat(tokens[2])));

		if (tokens.size() > 1 && tokens[0] == "usemtl")
		{
			// new group
			groups.push_back({ vertices.size(), indices.size() });
		}

		//daca am o fata (f+ minim 3 indecsi)
		if (tokens.size() >= 4 && tokens[0] == "f") {

			//foloseste primul vertex al fetei pentru a determina formatul fetei (v v/t v//n v/t/n) = (1 2 3 4)
			unsigned int face_format = 0;
			if (tokens[1].find("//") != std::string::npos) face_format = 3;
			faceTokenize(tokens[1], facetokens);
			if (facetokens.size() == 3) face_format = 4; // vertecsi/texcoords/normale
			else {
				if (facetokens.size() == 2) {
					if (face_format != 3) face_format = 2;	//daca nu am vertecsi/normale am vertecsi/texcoords
				}
				else {
					face_format = 1; //doar vertecsi
				}
			}

			//primul index din acest poligon
			unsigned int index_of_first_vertex_of_face = -1;


			for (unsigned int num_token = 1; num_token<tokens.size(); num_token++) {
				if (tokens[num_token].at(0) == '#') break;					//comment dupa fata
				faceTokenize(tokens[num_token], facetokens);
				if (face_format == 1) {
					//doar pozitie
					int p_index = stringToInt(facetokens[0]);
					if (p_index>0) p_index -= 1;								//obj has 1...n indices
					else p_index = positions.size() + p_index;				//index negativ

					vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z));
				}
				else if (face_format == 2) {
					// pozitie si texcoord
					int p_index = stringToInt(facetokens[0]);
					if (p_index>0) p_index -= 1;								//obj has 1...n indices
					else p_index = positions.size() + p_index;				//index negativ

					int t_index = stringToInt(facetokens[1]);
					if (t_index>0) t_index -= 1;								//obj has 1...n indices
					else t_index = texcoords.size() + t_index;				//index negativ

					vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z, texcoords[t_index].x, texcoords[t_index].y));
				}
				else if (face_format == 3) {
					//pozitie si normala
					int p_index = stringToInt(facetokens[0]);
					if (p_index>0) p_index -= 1;								//obj has 1...n indices
					else p_index = positions.size() + p_index;				//index negativ

					int n_index = stringToInt(facetokens[1]);
					if (n_index>0) n_index -= 1;								//obj has 1...n indices
					else n_index = normals.size() + n_index;					//index negativ

					vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z, normals[n_index].x, normals[n_index].y, normals[n_index].z));
				}
				else {
					//pozitie normala si texcoord
					int p_index = stringToInt(facetokens[0]);
					if (p_index>0) p_index -= 1;								//obj has 1...n indices
					else p_index = positions.size() + p_index;				//index negativ

					int t_index = stringToInt(facetokens[1]);
					if (t_index>0) t_index -= 1;								//obj has 1...n indices
					else t_index = normals.size() + t_index;					//index negativ

					int n_index = stringToInt(facetokens[2]);
					if (n_index>0) n_index -= 1;								//obj has 1...n indices
					else n_index = normals.size() + n_index;					//index negativ

					vertices.push_back(VertexFormat(positions[p_index].x, positions[p_index].y, positions[p_index].z, normals[n_index].x, normals[n_index].y, normals[n_index].z, texcoords[t_index].x, texcoords[t_index].y));
				}

				//adauga si indecsii
				if (num_token<4) {
					if (num_token == 1) index_of_first_vertex_of_face = vertices.size() - 1;
					//doar triunghiuri f 0 1 2 3 (4 indecsi, primul e ocupat de f)
					indices.push_back(vertices.size() - 1);
				}
				else {
					//polygon => triunghi cu ultimul predecesor vertexului nou adaugat si 0 relativ la vertecsi poligon(independent clockwise)
					indices.push_back(index_of_first_vertex_of_face);
					indices.push_back(vertices.size() - 2);
					indices.push_back(vertices.size() - 1);
				}
			}//end for
		}//end face

	}//end while
}

void loadNfgFile(const std::string &filename, std::vector<VertexFormat> &vertices, std::vector<unsigned int> &indices)
{
	string line;
	char lineBuff[LINE_BUFF];

	gzFile fp = gzopen(filename.c_str(), "r");

	if(!fp)
	{
		fprintf(stderr, "cannot open input file\n");
		exit(-1);
	}

	while (!gzeof(fp))
	{
		gzgets(fp, lineBuff, LINE_BUFF);

		if (lineBuff[0] == 0x0)
			continue;

		line = lineBuff;
		line.erase(line.find_last_not_of("\n\r") + 1);

		if (line.find("NrVertices") != string::npos)
		{
			size_t pos = line.find(':');

			if (pos == string::npos)
				break;

			string numStr = line.substr(pos + 1);
			vertices.reserve(atoi(numStr.c_str()));
		}
		else if (line.find("NrIndices") != string::npos)
		{
			size_t pos = line.find(':');

			if (pos == string::npos)
				break;

			string numStr = line.substr(pos + 1);
			indices.reserve(atoi(numStr.c_str()));
		}
		else if (line.find('[') != string::npos) // Vertex line
		{
			size_t pos = line.find('.');

			if (pos == string::npos)
				break;

			vertices.push_back(ReadVertex(line.substr(pos + 1)));
		}
		else // Index line
		{
			size_t pos = line.find('.');

			if (pos == string::npos)
				break;

			string indexLine = line.substr(pos + 1);
			vector<string> vec = SplitString(indexLine, ',');

			for (string s : vec)
				indices.push_back((unsigned short)atoi(s.c_str()));
		}
	}

	gzclose(fp);
}

void saveZfgFile(const string &filename, vector<VertexFormat> &vertices, vector<unsigned int> &indices, vector<GroupInfo> &groups)
{
	size_t group_id = 0;
	stringstream ss("", ios_base::app | ios_base::out);

	gzFile fp = gzopen(filename.c_str(), "w");
	//FILE *fp = fopen(filename.c_str(), "w");

	if (!fp)
	{
		printf("can't open output file\n");
		exit(-1);
	}

	ss << "NrVertices: " << vertices.size() << endl;

	for (size_t i = 0; i < vertices.size(); i++)
	{
		if (group_id < groups.size() && i == groups[group_id].vertices)
		{
			if (i != 0)
				ss << "NewVertexGroup" << endl;
			group_id++;
		}

		VertexFormat v = vertices.at(i);

		ss << i << ". pos:[";
		ss << v.position_x << "," << v.position_y << "," << v.position_z << "];norm:[";
		ss << v.normal_x << "," << v.normal_y << "," << v.normal_z << "];binorm:[";
		ss << v.binorm.x << "," << v.binorm.y << "," << v.binorm.z << "];tgt:[";
		ss << v.tgt.x << "," << v.tgt.y << "," << v.tgt.z << "];uv:[";
		ss << v.texcoord_x << "," << v.texcoord_y << "];" << endl;
	}

	ss << "NrIndices: " << indices.size() << endl;
	group_id = 0;
	size_t count = 0;
	int pos = 0;

	for (unsigned int i = 0; i < indices.size(); i++)
	{
		if (group_id < groups.size() && i == groups[group_id].indices)
		{
			if (i != 0)
				ss << "NewIndexGroup" << endl;
			group_id++;
		}

		if (pos == 0)
			ss << count << ". " << indices[i] << ",";
		else if (pos == 1)
			ss << indices[i] << ",";
		else if (pos == 2)
		{
			ss << indices[i] << endl;
			pos = -1;
			count++;
		}

		pos++;
	}

	/*fwrite(ss.str().c_str(), ss.str().size(), 1, fp);
	fclose(fp);*/
	gzwrite(fp, ss.str().c_str(), ss.str().size());
	gzclose(fp);
}

int main(int argc, char *argv[])
{
	vector<VertexFormat> vertices;
	vector<unsigned int> indices;
	vector<GroupInfo> groups;

	string inFile, outFile;
	bool compress = false;

	if (argc != 3)
	{
		if (argc == 2)
		{
			inFile = argv[1];
			outFile = argv[1];

			size_t pos = outFile.find(".obj");

			if (pos == string::npos)
			{
				pos = outFile.find(".nfg");

				if (pos == string::npos)
				{
					fprintf(stderr, "unknown input file format");
					return -1;
				}

				compress = true;
			}

			outFile.replace(pos, 4, ".zfg");
		}
		else if (argc == 4)
		{
			string str(argv[1]);

			if (str.find("compress") != string::npos)
				compress = true;
			else
				goto usage;

			inFile = argv[2];
			outFile = argv[3];
			compress = true;
		}
		else
			goto usage;
	}
	else
	{
		inFile = argv[1];
		outFile = argv[2];
	}

	printf("converting model...\n");

	if (compress)
		loadNfgFile(inFile, vertices, indices);
	else
		loadObjFile(inFile, vertices, indices, groups);

	saveZfgFile(outFile, vertices, indices, groups);

	printf("done\n");

	return 0;

usage:
	printf("usage: %s <obj file> <zfg file>\n", argv[0]);
	printf("usage: %s compress <uncompressed nfg file> <zfg file>\n", argv[0]);
	
	return 0;
}
