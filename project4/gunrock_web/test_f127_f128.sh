diskImageFile="test.img"
# ===== ===== =====
echo -e ">>> \033[94m0 Free Data Block left test running\033[0m"
./mkfs -f "$diskImageFile" -d 127 -i 136
echo -e ">>> test.img (127 blocks, 136 inodes) created"
echo -e ">>> Creating f1 to f126 (JUST WAIT)"
for i in $(seq 1 126); do
  dirName="f$i"
  ./ds3mkdir "$diskImageFile" 0 "$dirName"
done
echo -e ">>> Creating f127 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f127
echo -e ">>> Creating f128 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f128
echo -e ">>> Removing f127"
./ds3rm "$diskImageFile" 0 f127
echo -e ">>> Removing f128"
./ds3rm "$diskImageFile" 0 f128
echo -e ">>> \033[92m0 Free Data Block left test finished\033[0m"
# ===== ===== =====
echo -e ">>> \033[94m1 Free Data Block left test running\033[0m"
./mkfs -f "$diskImageFile" -d 128 -i 136
echo -e ">>> test.img (128 blocks, 136 inodes) created"
echo -e ">>> Creating f1 to f126 (JUST WAIT)"
for i in $(seq 1 126); do
  dirName="f$i"
  ./ds3mkdir "$diskImageFile" 0 "$dirName"
done
echo -e ">>> Creating f127 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f127
echo -e ">>> Creating f128 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f128
echo -e ">>> Removing f127"
./ds3rm "$diskImageFile" 0 f127
echo -e ">>> Removing f128"
./ds3rm "$diskImageFile" 0 f128
echo -e ">>> \033[92m1 Free Data Block left test finished\033[0m"
# ===== ===== =====
echo -e ">>> \033[94m2 Free Data Block left test running\033[0m"
./mkfs -f "$diskImageFile" -d 129 -i 136
echo -e ">>> test.img (129 blocks, 136 inodes) created"
echo -e ">>> Creating f1 to f126 (JUST WAIT)"
for i in $(seq 1 126); do
  dirName="f$i"
  ./ds3mkdir "$diskImageFile" 0 "$dirName"
done
echo -e ">>> Creating f127"
./ds3mkdir "$diskImageFile" 0 f127
echo -e ">>> Creating f128 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f128
echo -e ">>> Removing f127"
./ds3rm "$diskImageFile" 0 f127
echo -e ">>> Removing f128"
./ds3rm "$diskImageFile" 0 f128
echo -e ">>> \033[92m2 Free Data Block left test finished\033[0m"
# ===== ===== =====
echo -e ">>> \033[94m3 Free Data Block left test running\033[0m"
./mkfs -f "$diskImageFile" -d 130 -i 136
echo -e ">>> test.img (130 blocks, 136 inodes) created"
echo -e ">>> Creating f1 to f126 (JUST WAIT)"
for i in $(seq 1 126); do
  dirName="f$i"
  ./ds3mkdir "$diskImageFile" 0 "$dirName"
done
echo -e ">>> Creating f127"
./ds3mkdir "$diskImageFile" 0 f127
echo -e ">>> Creating f128"
./ds3mkdir "$diskImageFile" 0 f128
echo -e ">>> Removing f127"
./ds3rm "$diskImageFile" 0 f127
echo -e ">>> Removing f128"
./ds3rm "$diskImageFile" 0 f128
echo -e ">>> \033[92m3 Free Data Block left test finished\033[0m"


# ===== ===== =====
echo -e ">>> \033[94m0 Free Inode left test running\033[0m"
./mkfs -f "$diskImageFile" -d 136 -i 127
echo -e ">>> test.img (136 blocks, 127 inodes) created"
echo -e ">>> Creating f1 to f126 (JUST WAIT)"
for i in $(seq 1 126); do
  dirName="f$i"
  ./ds3mkdir "$diskImageFile" 0 "$dirName"
done
echo -e ">>> Creating f127 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f127
echo -e ">>> Creating f128 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f128
echo -e ">>> Removing f127"
./ds3rm "$diskImageFile" 0 f127
echo -e ">>> Removing f128"
./ds3rm "$diskImageFile" 0 f128
echo -e ">>> \033[92m0 Free Inode left test finished\033[0m"
# ===== ===== =====
echo -e ">>> \033[94m1 Free Inode left test running\033[0m"
./mkfs -f "$diskImageFile" -d 136 -i 128
echo -e ">>> test.img (136 blocks, 128 inodes) created"
echo -e ">>> Creating f1 to f126 (JUST WAIT)"
for i in $(seq 1 126); do
  dirName="f$i"
  ./ds3mkdir "$diskImageFile" 0 "$dirName"
done
echo -e ">>> Creating f127"
./ds3mkdir "$diskImageFile" 0 f127
echo -e ">>> Creating f128 (\033[31mmust throw error\033[0m)"
./ds3mkdir "$diskImageFile" 0 f128
echo -e ">>> Removing f127"
./ds3rm "$diskImageFile" 0 f127
echo -e ">>> Removing f128"
./ds3rm "$diskImageFile" 0 f128
echo -e ">>> \033[92m1 Free Inode left test finished\033[0m"
# ===== ===== =====
echo -e ">>> \033[94m2 Free Inode left test running\033[0m"
./mkfs -f "$diskImageFile" -d 136 -i 129
echo -e ">>> test.img (136 blocks, 129 inodes) created"
echo -e ">>> Creating f1 to f126 (JUST WAIT)"
for i in $(seq 1 126); do
  dirName="f$i"
  ./ds3mkdir "$diskImageFile" 0 "$dirName"
done
echo -e ">>> Creating f127"
./ds3mkdir "$diskImageFile" 0 f127
echo -e ">>> Creating f128"
./ds3mkdir "$diskImageFile" 0 f128
echo -e ">>> Removing f127"
./ds3rm "$diskImageFile" 0 f127
echo -e ">>> Removing f128"
./ds3rm "$diskImageFile" 0 f128
echo -e ">>> \033[92m2 Free Inode left test finished\033[0m"