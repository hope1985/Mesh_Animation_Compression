#pragma once

#include "GInclude.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <limits>

struct Vertex3f
{
	float x, y, z;
};
struct Vertex2f
{
	float x, y;
};

struct MeshAnimationInfo
{
	std::vector<std::vector<int>> faces;
	VectorX<VectorX<Vector3d>> vertices;
	std::vector<std::vector<Vertex3f>> verticesVertex3f;
	std::vector<std::vector<std::vector<double>>> verticesStd;
};
struct ObjFileData
{
	std::vector<Vertex3f> vPositins;
	std::vector<Vertex3f> vNormalizedPositins;
	std::vector<Vertex3f> vNorms;
	std::vector<Vertex3f> out_vPosLines;
	std::vector<Vertex3f> out_vPosLinesNormalized;
	//std::vector<Vertex3f> out_vPosFaces;
	std::vector<Vertex2f> vTextCoords;
	std::vector<int> face_pos_idxs;
	std::vector<int> face_pos_idxs_minus_1;
	std::vector<int> face_norm_idxs;
	std::vector<int> face_text_idxs;
	std::vector < std::vector<int>> faces3v_pos_idx;
	Vertex3f maxPoistionValues;
	Vertex3f minPoistionValues;
	float min_size, max_size;
	ObjFileData()
	{
		maxPoistionValues.x = -FLT_MAX;
		maxPoistionValues.y = -FLT_MAX;
		maxPoistionValues.z = -FLT_MAX;

		minPoistionValues.x = FLT_MAX;
		minPoistionValues.y = FLT_MAX;
		minPoistionValues.z = FLT_MAX;

		min_size = FLT_MAX;
		max_size = -FLT_MAX;
	}
};

static MeshAnimationInfo getMeshAnimationInfo(std::vector<ObjFileData> objFileMeshAnimationInfo, int frame_num)
{
	MeshAnimationInfo meshAnimationInfo;
	meshAnimationInfo.vertices.resize(frame_num);
	meshAnimationInfo.verticesVertex3f = std::vector<std::vector<Vertex3f>>(frame_num);
	meshAnimationInfo.verticesStd = std::vector<std::vector<std::vector<double>>>(frame_num);
	for (int k = 0; k < frame_num; k++)
	{
		ObjFileData mesh = objFileMeshAnimationInfo[k];

		VectorX<Vector3d> vertices(mesh.vPositins.size());
		std::vector<std::vector<double>> verticesStd(mesh.vPositins.size());

		//vertices.setZero();
		for (int v = 0; v < mesh.vPositins.size(); v++)
		{
			float x = mesh.vPositins[v].x;
			float y = mesh.vPositins[v].y;
			float z = mesh.vPositins[v].z;

			verticesStd[v].push_back(mesh.vPositins[v].x);
			verticesStd[v].push_back(mesh.vPositins[v].y);
			verticesStd[v].push_back(mesh.vPositins[v].z);

			vertices[v][0] = mesh.vPositins[v].x;
			vertices[v][1] = mesh.vPositins[v].y;
			vertices[v][2] = mesh.vPositins[v].z;
		}

		meshAnimationInfo.vertices[k] = vertices;
		meshAnimationInfo.faces = mesh.faces3v_pos_idx;
		meshAnimationInfo.verticesVertex3f[k] = mesh.vPositins;
		meshAnimationInfo.verticesStd[k] = verticesStd;
	}

	return meshAnimationInfo;
}


static  ObjFileData load_obj_file(const char* filename, int maxNumVertex = 999999999)
{
	ObjFileData mesh;
	std::vector<Vertex3f> vPos;
	std::vector<Vertex3f> out_vPos;
	std::vector<Vertex3f> vNorm;
	std::vector<Vertex2f> vText;
	std::vector<int> face_pos_idxs;
	std::vector<int> face_norm_idxs;
	std::vector<int> face_text_idxs;

	std::stringstream ss;
	std::ifstream ifile(filename);

	Vertex3f temp_vec3;
	int temp_GLint;

	if (!ifile.is_open())
	{
		throw "ERROR in load_obj_file method: Could not open file.";
	}
	std::string line = "";
	std::string line_type = "";

	while (std::getline(ifile, line))
	{
		ss.clear();
		ss.str(line);
		ss >> line_type;

		if (line_type == "v") //Position
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			if (vPos.size() < maxNumVertex)
			{
				vPos.push_back(Vertex3f(temp_vec3));

				if (temp_vec3.x < mesh.minPoistionValues.x)
					mesh.minPoistionValues.x = temp_vec3.x;
				if (temp_vec3.y < mesh.minPoistionValues.y)
					mesh.minPoistionValues.y = temp_vec3.y;
				if (temp_vec3.z < mesh.minPoistionValues.z)
					mesh.minPoistionValues.z = temp_vec3.z;

				if (temp_vec3.x > mesh.maxPoistionValues.x)
					mesh.maxPoistionValues.x = temp_vec3.x;
				if (temp_vec3.y > mesh.maxPoistionValues.y)
					mesh.maxPoistionValues.y = temp_vec3.y;
				if (temp_vec3.z > mesh.maxPoistionValues.z)
					mesh.maxPoistionValues.z = temp_vec3.z;

				float size = sqrt((temp_vec3.x * temp_vec3.x) + (temp_vec3.y * temp_vec3.y) + (temp_vec3.z * temp_vec3.z));
				if (size < mesh.min_size)
				{
					mesh.min_size = size;
				}
				if (size > mesh.max_size)
				{
					mesh.max_size = size;
				}
			}
		}
		else if (line_type == "vn")
		{
			ss >> temp_vec3.x >> temp_vec3.y >> temp_vec3.z;
			vNorm.push_back(Vertex3f(temp_vec3));
		}
		else if (line_type == "f")  //[f v/vt/vn] OR [f v/vt] OR [f v//vn] OR [f v]
		{
			int cnt = 0;
			while (ss >> temp_GLint)
			{
				if (cnt == 0)
					face_pos_idxs.push_back(temp_GLint);
				else if (cnt == 1)
					face_text_idxs.push_back(temp_GLint);
				else if (cnt == 2)
					face_norm_idxs.push_back(temp_GLint);

				if (ss.peek() == '/')
				{
					cnt++;
					ss.ignore(1, '/');
					if (ss.peek() == '/')  //Support [f v//vn]
					{
						cnt++;
						ss.ignore(1, '/');
					}
				}
				else if (ss.peek() == ' ')
				{
					if (cnt == 0 || cnt == 1)  //Support [f v] (cnt=0)  OR  Support [f v/vt] (cnt=1)
						cnt = 3;
					else
						cnt++;
					ss.ignore(1, ' ');
				}

				if (cnt > 2)
					cnt = 0;
			}

		}
	}
	mesh.vPositins = vPos;
	mesh.vNorms = vNorm;
	mesh.face_pos_idxs = std::vector<int>(); //face_pos_idxs;
	mesh.face_pos_idxs_minus_1 = std::vector<int>();
	mesh.face_norm_idxs = face_norm_idxs;
	mesh.faces3v_pos_idx = std::vector<std::vector<int>>();
	mesh.vNormalizedPositins = std::vector<Vertex3f>();


	for (int i = 0; i<int(mesh.vPositins.size()); i++)
	{
		Vertex3f p;
		p.x = (mesh.vPositins[i].x / mesh.max_size);
		p.y = (mesh.vPositins[i].y / mesh.max_size);
		p.z = (mesh.vPositins[i].z / mesh.max_size);
		mesh.vNormalizedPositins.push_back(p);
	}

	for (int i = 0; i<int(face_pos_idxs.size() / 3); i++)
	{

		if (face_pos_idxs[3 * i] <= maxNumVertex && face_pos_idxs[3 * i + 1] <= maxNumVertex && face_pos_idxs[3 * i + 2] <= maxNumVertex)
		{
			mesh.faces3v_pos_idx.push_back(std::vector<int>());
			mesh.faces3v_pos_idx[mesh.faces3v_pos_idx.size() - 1].push_back(face_pos_idxs[3 * i]);
			mesh.faces3v_pos_idx[mesh.faces3v_pos_idx.size() - 1].push_back(face_pos_idxs[3 * i + 1]);
			mesh.faces3v_pos_idx[mesh.faces3v_pos_idx.size() - 1].push_back(face_pos_idxs[3 * i + 2]);

			mesh.face_pos_idxs.push_back(face_pos_idxs[3 * i]);
			mesh.face_pos_idxs.push_back(face_pos_idxs[3 * i + 1]);
			mesh.face_pos_idxs.push_back(face_pos_idxs[3 * i + 2]);

			mesh.face_pos_idxs_minus_1.push_back(face_pos_idxs[3 * i] - 1);
			mesh.face_pos_idxs_minus_1.push_back(face_pos_idxs[3 * i + 1] - 1);
			mesh.face_pos_idxs_minus_1.push_back(face_pos_idxs[3 * i + 2] - 1);
		}
	}

	for (int i = 0; i<int(face_pos_idxs.size() / 3); i++)
	{

		if (face_pos_idxs[3 * i] <= maxNumVertex && face_pos_idxs[3 * i + 1] <= maxNumVertex && face_pos_idxs[3 * i + 2] <= maxNumVertex)
		{
			int val1 = (int)face_pos_idxs[3 * i];
			int val2 = (int)face_pos_idxs[3 * i + 1];
			int val3 = (int)face_pos_idxs[3 * i + 2];

			//mesh.out_vPosFaces.push_back(mesh.vPositins[val1 - 1]);
			//mesh.out_vPosFaces.push_back(mesh.vPositins[val2 - 1]);
			//mesh.out_vPosFaces.push_back(mesh.vPositins[val3 - 1]);
			//============================
			mesh.out_vPosLines.push_back(mesh.vPositins[val1 - 1]);

			mesh.out_vPosLines.push_back(mesh.vPositins[val2 - 1]);
			mesh.out_vPosLines.push_back(mesh.vPositins[val2 - 1]);

			mesh.out_vPosLines.push_back(mesh.vPositins[val3 - 1]);

			//============================
			mesh.out_vPosLinesNormalized.push_back(mesh.vNormalizedPositins[val1 - 1]);

			mesh.out_vPosLinesNormalized.push_back(mesh.vNormalizedPositins[val2 - 1]);
			mesh.out_vPosLinesNormalized.push_back(mesh.vNormalizedPositins[val2 - 1]);

			mesh.out_vPosLinesNormalized.push_back(mesh.vNormalizedPositins[val3 - 1]);
		}
	}
	return mesh;

};

static void save_obj_file(const char* filename, std::vector<std::vector<int>> faces, std::vector<std::vector<Vertex3f>> vertices)
{

	std::stringstream ss;
	std::ofstream ofile(filename);
	if (!ofile.is_open())
	{
		throw "ERROR in save_obj_file method: Could not careat file.";
	}

	ss << "# Compressed frame" << std::endl;
	for (int i = 0; i < vertices[0].size(); i++)
	{
		ss << "v" << "   " << std::setprecision(9) << vertices[0][i].x << "    " << std::setprecision(9) << vertices[0][i].y << "    " << std::setprecision(9) << vertices[0][i].z << std::endl;
	}
	for (int i = 0; i < faces.size(); i++)
	{
		ss << "f" << "   " << faces[i][0] << "    " << faces[i][1] << "    " << faces[i][2] << std::endl;
	}
	ofile << ss.str();
	ofile.close();
}
static void save_obj_file(const char* filename, std::vector<std::vector<int>> faces, std::vector<std::vector<std::vector<double>>> vertices, int fnum = 0)
{

	std::stringstream ss;
	std::ofstream ofile(filename);
	if (!ofile.is_open())
	{
		throw "ERROR in save_obj_file method: Could not careat file.";
	}

	ss << "# Compressed frame" << std::endl;
	for (int i = 0; i < vertices[fnum].size(); i++)
	{
		ss << "v" << "   " << vertices[fnum][i][0] << "    " << vertices[fnum][i][1] << "    " << vertices[fnum][i][2] << std::endl;
	}
	for (int i = 0; i < faces.size(); i++)
	{
		ss << "f" << "   " << faces[i][0] << "    " << faces[i][1] << "    " << faces[i][2] << std::endl;
	}
	ofile << ss.str();
	ofile.close();
}


static std::vector<ObjFileData> load_mesh_files(string rootdir, std::vector<string> filenames, int fn, int maxVertex)
{

	fn = fn <= filenames.size() ? fn : (int)filenames.size();

	string* frames = new string[fn];
	std::vector<ObjFileData> mesh_frames;
	std::vector<glm::vec3> allframe(fn);
	for (int k = 0; k < fn; k++)
	{
		auto mesh = load_obj_file((rootdir + "\\" + filenames[k]).c_str(), maxVertex);
		mesh_frames.push_back(mesh);
		for (int l = 0; l < mesh.out_vPosLines.size(); l++)
		{
			allframe[k] = glm::vec3(mesh.out_vPosLines[l].x, mesh.out_vPosLines[l].y, mesh.out_vPosLines[l].z);
		}
	}
	std::cout << "objec max coord=(" << mesh_frames[0].maxPoistionValues.x << "," << mesh_frames[0].maxPoistionValues.x << "," << mesh_frames[0].maxPoistionValues.z << ")    ";
	std::cout << "objec min coord=(" << mesh_frames[0].minPoistionValues.x << "," << mesh_frames[0].minPoistionValues.x << "," << mesh_frames[0].minPoistionValues.z << ")    " << "\n";

	return mesh_frames;
}

