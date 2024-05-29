import os
import os.path as osp



def makedirs(path):
	if not osp.exists(path):
		os.makedirs(path)
	else:
		print('this file is already exists.')


if __name__=="__main__":
	dst_path = '/home/bitwrj/SpireCV/dataset/MOT17'
	mode = 'train'
	dst_file_name = 'val_seqmap.txt'
	dst_file = osp.join(dst_path,dst_file_name)
	test_files = os.listdir(osp.join(dst_path,mode))
	test_files.sort()
	with open(dst_file, 'w+') as f:
		f.writelines('name\n')
		for test_file in test_files:
			f.writelines(test_file+'\n')
