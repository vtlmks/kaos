#ifndef __INODE_H__
#define __INODE_H__

struct Inode {
	u16	i_mode;
	u16	i_uid;
	u32	i_size;
	u32	i_atime;
	u32	i_ctime;
	u32	i_mtime;
	u32	i_dtime;
	u16	i_gid;
	u16	i_links_count;
	u32	i_blocks;
	u32	i_flags;
	u32	i_osd1;
	u32	i_block[15];	// 60
	u32	i_generation;
	u32	i_file_acl;
	u32	i_dir_acl;
	u32	i_faddr;
	u32	i_osd2[3];	// 12
};


#define	EXT2_S_IXOTH	1 << 0
#define	EXT2_S_IWOTH	1 << 1
#define	EXT2_S_IROTH	1 << 2
#define	EXT2_S_IXGRP	1 << 5
#define	EXT2_S_IWGRP	1 << 4
#define	EXT2_S_IRGRP	1 << 3
#define	EXT2_S_IXUSR	1 << 6
#define	EXT2_S_IWUSR	1 << 7
#define	EXT2_S_IRUSR	1 << 8

#define	EXT2_S_ISUID	1 << 9
#define	EXT2_S_ISGID	1 << 10
#define	EXT2_S_ISVTX	1 << 11

#define	EXT2_S_IFIFO	1 << 12
#define	EXT2_S_IFCHR	1 << 13
#define	EXT2_S_IFDIR	1 << 14
#define	EXT2_S_IFBLK	1 << 15
#define	EXT2_S_IFREG	1 << 16
#define	EXT2_S_IFLNK	1 << 17
#define	EXT2_S_ISOCK	1 << 18

#endif

