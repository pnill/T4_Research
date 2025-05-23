#ifndef __ANIMTBLEDDEFS_H__
#define __ANIMTBLEDDEFS_H__

#define SIZE_ID			96
#define SIZE_FILENAME	256

#define ANIMTREENODE_TYPE_UNDEF		0
#define ANIMTREENODE_TYPE_PROJECT	1
#define ANIMTREENODE_TYPE_ENUM		2
#define ANIMTREENODE_TYPE_UNUSED	3
#define ANIMTREENODE_TYPE_GROUP		4
#define ANIMTREENODE_TYPE_ANIM		5
#define	ANIMTREENODE_TYPE_EQUIV		6	// <<--- These were removed from the tree.  Place holder for old save files
#define ANIMTREENODE_TYPE_TRANS		7


enum
{
	TREEIMAGE_PROJECT,
	TREEIMAGE_PROJECT_SELECT,
	TREEIMAGE_ENUM,
	TREEIMAGE_ENUM_SELECT,
	TREEIMAGE_UNUSED,
	TREEIMAGE_UNUSED_SELECT,
	TREEIMAGE_GROUP,
	TREEIMAGE_GROUP_SELECT,
	TREEIMAGE_ANIM,
	TREEIMAGE_ANIM_LINKED,
	TREEIMAGE_ANIM_SELECT,
	TREEIMAGE_ANIM_WITHDEP,
	TREEIMAGE_ANIM_LINKED_WITHDEP,
	TREEIMAGE_ANIM_SELECT_WITHDEP,
	TREEIMAGE_EQUIV,
	TREEIMAGE_EQUIV_SELECT,
	TREEIMAGE_TRANS,
	TREEIMAGE_TRANS_SELECT,
	TREEIMAGE_PREREQ,
	TREEIMAGE_PREREQ_SELECT
};

#define TOKEN_NONE			0
#define TOKEN_GROUPSTART	1
#define TOKEN_GROUPEND		2
#define TOKEN_ANIM			3
#define	TOKEN_EQUIV			4

#define EXPORTTYPE_ANIMLIST				1
#define EXPORTTYPE_ANIMLISTGROUP		2
#define	EXPORTTYPE_NUMANIMS				3
#define EXPORTTYPE_MACROS				4
#define EXPORTTYPE_ANIMOBJECTS			5
#define EXPORTTYPE_DEPENDANCIES			6
#define	EXPORTTYPE_EXTRAINFO			7
#define	EXPORTTYPE_OBJECTLIST			8
#define	EXPORTTYPE_OBJECTLISTGROUP		9
#define	EXPORTTYPE_LOCOMOTIONANIMARRAY	10

#define	DROPTARGET_INSERTBEFORE		(1<<0)
#define	DROPTARGET_INSERTINSIDE		(1<<1)
#define	DROPTARGET_INSERTCOPY		(1<<2)


#define LISTBOX_FLAG_INCLUDEANIMS	(1<<0)
#define LISTBOX_FLAG_INCLUDEGROUPS	(1<<1)


#endif // __ANIMTBLEDDEFS_H__