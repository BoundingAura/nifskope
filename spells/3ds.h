#ifndef IMPORT_3DS_H
#define IMPORT_3DS_H

// Chunk Type definitions
#define NULL_CHUNK			0x0000
#define M3D_VERSION			0x0002
#define M3D_KFVERSION		0x0003
#define	COLOR_F				0x0010
#define COLOR_24			0x0011
#define LIN_COLOR_24		0x0012
#define LIN_COLOR_F			0x0013
#define INT_PERCENTAGE		0x0030
#define FLOAT_PERCENTAGE	0x0031
#define MASTER_SCALE		0x0100

#define DEFAULT_VIEW		0x3000
#define VIEW_TOP			0x3010
#define VIEW_BOTTOM			0x3020
#define VIEW_LEFT			0x3030
#define VIEW_RIGHT			0x3040
#define VIEW_FRONT			0x3050
#define VIEW_BACK			0x3060
#define VIEW_USER			0x3070
#define VIEW_CAMERA			0x3080
#define MDATA				0x3D3D
#define MESH_VERSION		0x3D3E

#define NAMED_OBJECT		0x4000
#define N_TRI_OBJECT		0x4100
#define POINT_ARRAY			0x4110
#define POINT_FLAG_ARRAY	0x4111
#define FACE_ARRAY			0x4120
#define MSH_MAT_GROUP		0x4130
#define TEX_VERTS			0x4140
#define SMOOTH_GROUP		0x4150
#define MESH_MATRIX			0x4160
#define MESH_COLOR			0x4165
#define M3DMAGIC			0x4D4D

#define MAT_NAME			0xA000
#define MAT_AMBIENT			0xA010
#define MAT_DIFFUSE			0xA020
#define MAT_SPECULAR		0xA030
#define MAT_SHININESS		0xA040
#define MAT_SHIN2PCT		0xA041
#define MAT_SHIN3PCT		0xA042
#define MAT_TRANSPARENCY	0xA050
#define MAT_XPFALL			0xA052
#define MAT_REFBLUR			0xA053
#define MAT_SELF_ILLUM		0xA080
#define MAT_TWOSIDE			0xA081
#define MAT_SELF_ILPCT		0xA084
#define MAT_WIRESIZE		0xA087
#define MAT_XPFALLIN		0xA08A
#define MAT_SHADING			0xA100
#define MAT_TEXMAP			0xA200
#define MAT_SPECMAP			0xA204
#define MAT_OPACMAP			0xA210
#define MAT_REFLMAP			0xA220
#define MAT_BUMPMAP			0xA230
#define MAT_MAPNAME			0xA300
#define MATERIAL			0xAFFF

#define KEYF3DS				0xB000
#define OBJECT_NODE_TAG		0xB002
#define KFSEG				0xB008
#define NODE_HDR			0xB010
#define PIVOT				0xB013
#define POS_TRACK_TAG		0xB020
#define ROT_TRACK_TAG		0xB021
#define SCL_TRACK_TAG		0xB022
#define FOV_TRACK_TAG		0xB023
#define ROLL_TRACK_TAG		0xB024
#define COL_TRACK_TAG		0xB025
#define MORPH_TRACK_TAG		0xB026
#define HOT_TRACK_TAG		0xB027
#define FALL_TRACK_TAG		0xB028
#define HIDE_TRACK_TAG		0xB029
#define NODE_ID				0xB030

#define CHUNKHEADERSIZE 6
#define FILE_DUMMY 0xFFFF

#include <QFile>
#include <QList>
#include <QMap>
#include <QString>

class Chunk {
public:

	// data structures for various chunks

	typedef float ChunkTypeFloat;

	struct ChunkTypeFloat2
	{
		float x, y;
	};

	struct ChunkTypeFloat3
	{
		float x, y, z;
	};

	typedef short ChunkTypeShort;

	struct ChunkTypeFaceArray
	{
	  	short vertex1, vertex2, vertex3;
		short flags;
	};

	struct ChunkTypeMeshMatrix
	{
		float matrix[4][3];
	};

	// general chunk properties

	typedef unsigned short	ChunkType;
	typedef unsigned int	ChunkPos;
	typedef unsigned long	ChunkLength;
	typedef unsigned char	ChunkData;
	typedef bool			ChunkDataFlag;
	typedef unsigned int	ChunkDataLength;
	typedef unsigned short	ChunkDataCount;

	struct ChunkHeader
	{
		ChunkType	t;
		ChunkLength	l;
	};

	// class members

	Chunk( QFile * _f, ChunkHeader _h, ChunkPos _p )
		: f( _f ), h( _h ), p( _p ), d( NULL ), df( false ), dl( 0 ), dc( 0 )
	{
		if( h.t == FILE_DUMMY ) {
			this->addchildren();
		}
		else {
			subproc();
		}
	}

	~Chunk()
	{
		clearData();
		qDeleteAll( c );
	}

	static Chunk * LoadFile( QFile * file ) {
		file->seek( 0 );

		ChunkHeader hdr;
		hdr.t = FILE_DUMMY;
		hdr.l = file->size();

		Chunk * cnk = new Chunk( file, hdr, ( - CHUNKHEADERSIZE ) );

		return cnk;
	}

	ChunkData * getData()
	{
		if( df && ( d == NULL ) ) {
			readdata();
		}

		return d;
	}

	QList< Chunk * > getChildren( ChunkType ct = NULL_CHUNK )
	{
		if( ct == NULL_CHUNK ) {
			return c.values();
		}

		return c.values( ct );
	}

	Chunk * getChild( ChunkType ct )
	{
		return c[ct];
	}

	void clearData()
	{
		delete[] d;
	}

	QMap< ChunkType, Chunk * >  children();

private:
	ChunkHeader		h;
	ChunkPos		p;
	ChunkData *		d;
	ChunkDataFlag	df;
	ChunkDataLength	dl;
	ChunkDataCount	dc;

	QFile * f;
	QMap< ChunkType, Chunk * > c;

	void subproc() {
		switch( h.t & 0xf000 ) {
			case 0x0000:

				switch( h.t )
				{
					case M3D_VERSION:
					case M3D_KFVERSION:
						adddata( sizeof( ChunkTypeShort ) );
						break;

					case COLOR_F:
					case LIN_COLOR_F:
						adddata( sizeof( ChunkTypeFloat3 ) );
						break;

					case COLOR_24:
					case LIN_COLOR_24:
						adddata();
						break;

					case INT_PERCENTAGE:
						adddata( sizeof( ChunkTypeShort ) );
						break;

					case FLOAT_PERCENTAGE:
						adddata( sizeof( ChunkTypeFloat ) );
						break;

					case MASTER_SCALE:
						adddata( sizeof( ChunkTypeFloat ) );
						break;
				}

				break;

			case 0x3000:

				switch( h.t )
				{
					case DEFAULT_VIEW:
						addchildren();
						break;

					case VIEW_TOP:
					case VIEW_BOTTOM:
					case VIEW_LEFT:
					case VIEW_RIGHT:
					case VIEW_FRONT:
					case VIEW_BACK:
					case VIEW_USER:
						adddata();
						break;

					case VIEW_CAMERA:
						addname();
						adddata();
						break;

					case MDATA:
						addchildren();
						break;
				}

				break;

			case 0x4000:

				switch( h.t )
				{
					case NAMED_OBJECT:
						addname();
						adddata();
						addchildren();
						break;

					case N_TRI_OBJECT:
						addchildren();
						break;

					case POINT_ARRAY:
						addcount( sizeof( ChunkTypeFloat3 ) );
						adddata();
						break;

					case POINT_FLAG_ARRAY:
						addcount( sizeof( ChunkTypeShort ) );
						adddata();
						break;

					case FACE_ARRAY:
						addcount( sizeof( ChunkTypeFaceArray ) );
						adddata();
						break;

					case MSH_MAT_GROUP:
						addname();
						addcount( sizeof( ChunkTypeShort ) );
						adddata();
						break;

					case TEX_VERTS:
						addcount( sizeof( ChunkTypeFloat2 ) );
						adddata();
						break;

					case MESH_MATRIX:
						adddata( sizeof( ChunkTypeMeshMatrix ) );
						break;

					case M3DMAGIC:
						addchildren();
						break;

				}

				break;

			case 0xa000:

				switch( h.t ) {

					case MAT_NAME:
						addname();
						adddata();
						break;

					case MAT_AMBIENT:
					case MAT_DIFFUSE:
					case MAT_SPECULAR:
						addchildren();
						break;

					case MAT_SHININESS:
					case MAT_SHIN2PCT:
					case MAT_SHIN3PCT:
					case MAT_TRANSPARENCY:
					case MAT_XPFALL:
					case MAT_REFBLUR:
					case MAT_SELF_ILPCT:
						addchildren();
						break;

					case MAT_WIRESIZE:
						adddata( sizeof( ChunkTypeFloat ) );
						break;

					case MAT_SHADING:
						adddata( sizeof( ChunkTypeShort) );
						break;

					case MAT_TEXMAP:
					case MAT_SPECMAP:
					case MAT_OPACMAP:
					case MAT_REFLMAP:
					case MAT_BUMPMAP:
						addchildren();
						break;

					case MAT_MAPNAME:
						addname();
						adddata();
						break;

					case MATERIAL:
						addchildren();
						break;
				}

				break;

			case 0xb000:
				break;
		}
	}

	void addchildren()
	{
		f->seek( p + CHUNKHEADERSIZE + dl );

		while( f->pos() < ( p + h.l ) )
		{
			ChunkPos q = f->pos();

			ChunkHeader k;
			f->read( (char *)( &k.t ), sizeof( k.t ) );
			f->read( (char *)( &k.l ), sizeof( k.l ) );

			Chunk * z = new Chunk( f, k, q );
			
			c.insertMulti( k.t, z );

			f->seek( q + k.l );
		}

		f->seek( p + h.l );
	}

	void addname()
	{
		f->seek( p + CHUNKHEADERSIZE + dl );

		char n = 0x01;
		int nl = 0;

		while( n != NULL )
		{
			if( !f->getChar( &n ) ) {
				break;
			}

			nl++;
		}

		dl += nl;
	}

	void addcount( ChunkDataLength _dl )
	{
		f->seek( p + CHUNKHEADERSIZE + dl );

		f->read( (char *)( dc ), sizeof( dc ) );

		dl += ( sizeof( ChunkDataCount ) + ( dc * _dl ) );
	}


	void adddata( ChunkDataLength _dl = 0 )
	{
		f->seek( p + CHUNKHEADERSIZE );

		dl += _dl;

		if( dl == 0 ) {
			dl = ( h.l - CHUNKHEADERSIZE ) / sizeof( ChunkData );
		}

		Q_ASSERT( ( dl % sizeof( ChunkData ) ) == 0 );

		df = true;

		f->seek( p + CHUNKHEADERSIZE + dl );
	}

	void readdata()
	{
		d = new ChunkData[ dl / sizeof( ChunkData ) ];

		f->seek( p + CHUNKHEADERSIZE );

		f->read( (char *)( d ), dl );
		
		f->seek( p + dl );
	}

};

#endif
