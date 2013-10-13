#ifndef LOADOBJ_H
#define LOADOBJ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h> // size_t

typedef struct {
	float x, y, z;
} LoadOBJVertexPosition;

typedef struct {
	float u, v;
} LoadOBJVertexTexCoord;

typedef struct {
	float x, y, z;
} LoadOBJVertexNormal;

typedef struct {
	long long v, vt, vn;
} LoadOBJVertexIndex;

typedef struct {
	LoadOBJVertexIndex *indices;
	size_t indexCount;
	size_t _indexCapacity;
} LoadOBJFace;

typedef struct {
	LoadOBJVertexPosition *positions;
	size_t positionCount;
	LoadOBJVertexTexCoord *texCoords;
	size_t texCoordCount;
	LoadOBJVertexNormal *normals;
	size_t normalCount;
	LoadOBJFace *faces;
	size_t faceCount;
} LoadOBJMesh;

void loadOBJ(LoadOBJMesh *mesh, const char *string);
void loadOBJDestroyMesh(LoadOBJMesh *mesh);

#ifdef LOADOBJ_IMPLEMENTATION

#include <stdlib.h> // realloc, free, strtof, strtoll
#include <string.h> // memset, strlen, strncmp, strchr, strspn
#include <ctype.h>  // isspace

#ifdef LOADOBJ_LOG_UNSUPPORTED
#   include <stdio.h> // printf
#endif

#define _LOADOBJ_ARRAY(type) \
	struct { size_t count, capacity; type *data; }

#define _LOADOBJ_ARRAY_GROW(type, arr) \
	((arr).capacity = (arr).capacity ? ((arr).capacity << 1) : 2, \
	(arr).data = (type*) realloc((arr).data, (arr).capacity * sizeof(type)))

void loadOBJ(LoadOBJMesh *mesh, const char *string)
{
	_LOADOBJ_ARRAY(LoadOBJVertexPosition) vertexPositions;
	_LOADOBJ_ARRAY(LoadOBJVertexTexCoord) vertexTexCoords;
	_LOADOBJ_ARRAY(LoadOBJVertexNormal)   vertexNormals;
	_LOADOBJ_ARRAY(LoadOBJFace) faces;

	memset(&vertexPositions, 0, sizeof(vertexPositions));
	memset(&vertexTexCoords, 0, sizeof(vertexTexCoords));
	memset(&vertexNormals,   0, sizeof(vertexNormals));
	memset(&faces, 0, sizeof(faces));

	const char *line = string;
	const char *nextLine;

	for (;;)
	{
		line += strspn(line, " \t\n\r");

		if (*line == '\0')
			break;

		nextLine = strchr(line, '\n');

		if (*line == '#')
		{
			if (nextLine == NULL)
				break;

			line = nextLine + 1;

			continue;
		}
		else if ((line[0] == 'v') && isspace(line[1]))
		{
			line += 1;

			if (vertexPositions.count == vertexPositions.capacity)
				_LOADOBJ_ARRAY_GROW(LoadOBJVertexPosition, vertexPositions);

			vertexPositions.data[vertexPositions.count].x = strtof(line, (char**)&line);
			vertexPositions.data[vertexPositions.count].y = strtof(line, (char**)&line);
			vertexPositions.data[vertexPositions.count].z = strtof(line, (char**)&line);
			++vertexPositions.count;
		}
		else if (!strncmp(line, "vt", 2) && isspace(line[2]))
		{
			line += 2;

			if (vertexTexCoords.count == vertexTexCoords.capacity)
				_LOADOBJ_ARRAY_GROW(LoadOBJVertexTexCoord, vertexTexCoords);

			vertexTexCoords.data[vertexTexCoords.count].u = strtof(line, (char**)&line);
			vertexTexCoords.data[vertexTexCoords.count].v = strtof(line, (char**)&line);
			++vertexTexCoords.count;
		}
		else if (!strncmp(line, "vn", 2) && isspace(line[2]))
		{
			line += 2;

			if (vertexNormals.count == vertexNormals.capacity)
				_LOADOBJ_ARRAY_GROW(LoadOBJVertexNormal, vertexNormals);

			vertexNormals.data[vertexNormals.count].x = strtof(line, (char**)&line);
			vertexNormals.data[vertexNormals.count].y = strtof(line, (char**)&line);
			vertexNormals.data[vertexNormals.count].z = strtof(line, (char**)&line);
			++vertexNormals.count;
		}
		else if ((line[0] == 'f') && isspace(line[1]))
		{
			line += 1;

			if (faces.count == faces.capacity)
				_LOADOBJ_ARRAY_GROW(LoadOBJFace, faces);

			LoadOBJFace *face = faces.data + faces.count;

			memset(face, 0, sizeof(LoadOBJFace));

			for (;;)
			{
				if (face->indexCount == face->_indexCapacity)
				{
					face->_indexCapacity = face->_indexCapacity ? (face->_indexCapacity << 1) : 2;
					face->indices = (LoadOBJVertexIndex*) realloc(face->indices, face->_indexCapacity * sizeof(LoadOBJVertexIndex));
				}

				face->indices[face->indexCount].v = strtoll(line, (char**)&line, 10);

				if (*line == '/')
				{
					++line;

					if (*line != '/')
						face->indices[face->indexCount].vt = strtoll(line, (char**)&line, 10);
					else
						face->indices[face->indexCount].vt = 0;

					if (*line == '/')
					{
						++line;
						face->indices[face->indexCount].vn = strtoll(line, (char**)&line, 10);
					}
					else
						face->indices[face->indexCount].vn = 0;
				}
				else
				{
					face->indices[face->indexCount].vt = 0;
					face->indices[face->indexCount].vn = 0;
				}

				++face->indexCount;

				line += strspn(line, " \t\r");

				if ((*line == '\n') || (*line == '\0'))
					break;
			}

			++faces.count;
		}
#ifdef LOADOBJ_LOG_UNSUPPORTED
		else
			printf("Unsupported: %.*s\n", nextLine ? (int) (nextLine - line) : (int) strlen(line), line);
#endif

		if (nextLine == NULL)
			break;

		line = nextLine + 1;
	}

	mesh->positionCount = vertexPositions.count;
	mesh->positions = (LoadOBJVertexPosition*) realloc(vertexPositions.data, mesh->positionCount * sizeof(LoadOBJVertexPosition));

	mesh->texCoordCount = vertexTexCoords.count;
	mesh->texCoords = (LoadOBJVertexTexCoord*) realloc(vertexTexCoords.data, mesh->texCoordCount * sizeof(LoadOBJVertexTexCoord));

	mesh->normalCount = vertexNormals.count;
	mesh->normals = (LoadOBJVertexNormal*) realloc(vertexNormals.data, mesh->normalCount * sizeof(LoadOBJVertexNormal));

	mesh->faceCount = faces.count;
	mesh->faces = (LoadOBJFace*) realloc(faces.data, mesh->faceCount * sizeof(LoadOBJFace));
}

void loadOBJDestroyMesh(LoadOBJMesh *mesh)
{
	free(mesh->positions);
	free(mesh->texCoords);
	free(mesh->normals);

	for (size_t i = 0; i < mesh->faceCount; ++i)
		free(mesh->faces[i].indices);

	free(mesh->faces);
}

#endif

#ifdef __cplusplus
}
#endif

#endif