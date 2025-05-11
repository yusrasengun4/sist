#include "fs.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


#include <sys/types.h>


#define METADATA_SIZE 4096
#define RECORD_SIZE  264
#define MAX_FILENAME_LEN 256

int disk_fd = -1;





int disk_fd = -1;

int fs_init() {
    // disk.sim dosyasını aç (yoksa oluştur, okuma ve yazma izni ver)
    disk_fd = open("disk.sim", O_RDWR | O_CREAT, 0644);
    
    if (disk_fd == -1) {
        perror("fs_init: disk.sim açılamadı");
        return -1;
    }

    // Dosya boyutunu kontrol et, eğer disk boşsa (ilk defa başlatılıyorsa)
    struct stat st;
    if (fstat(disk_fd, &st) == 0) {
        if (st.st_size == 0) {
            // Dosya boyutu sıfırsa (yeni dosya veya formatlanmamış)
            // Disk alanını ayarla (örneğin 1MB disk simülasyonu oluşturuluyor)
            char zero = 0; // Diskin başını '0' byte ile doldurmak için
            for (int i = 0; i < 1024 * 1024; i++) { // 1MB'lık boş alan oluştur
                write(disk_fd, &zero, sizeof(zero));
            }
            printf("Yeni disk.sim dosyası oluşturuldu (1MB).\n");
        } else {
            printf("disk.sim dosyası mevcut ve kullanılabilir.\n");
        }
    } else {
        perror("Dosya boyutu kontrol edilemedi");
        close(disk_fd);
        return -1;
    }

    return 0;
}

void fs_close() {
    if (disk_fd != -1) {
        close(disk_fd);
        disk_fd = -1;
    }
}

int check_filename(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        perror("Geçersiz dosya adı");
        return -1;
    }
    return 0;
}


void fs_close() {
    if (disk_fd != -1) {
        close(disk_fd);
        disk_fd = -1;
    }
}

int fs_create(char *filename) {
     if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;

    lseek(disk_fd, 0, SEEK_SET);

    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    // Önce dosya zaten var mı diye kontrol ediyoruz
    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            printf("fs_create: Dosya zaten var: %s\n", filename);
			fs_log("CREATE", filename, "FAILED: Dosya zaten var");
            return -1;
        }
    }

    // Eğer dosya yoksa, boş yer arayıp kaydet
    lseek(disk_fd, 0, SEEK_SET); // Baştan tekrar başla

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (name_buf[0] == '\0') { // boş kayıt bulduk
            lseek(disk_fd, -RECORD_SIZE, SEEK_CUR);
            write(disk_fd, filename, MAX_FILENAME_LEN);
            int new_filesize = 0;
            int new_offset = METADATA_SIZE + (i * 4096); // her dosya 4KB veri bloğu kaplar
            write(disk_fd, &new_filesize, sizeof(int));
            write(disk_fd, &new_offset, sizeof(int));
            printf("fs_create: Dosya oluşturuldu: %s\n", filename);
			 fs_log("CREATE", filename, "SUCCESS");
            return 0;
        }
    }

    printf("fs_create: Yer kalmadı!\n");
	fs_log("CREATE", filename, "FAILED: Yer kalmadi");
    return -1;
}
int fs_write(char *filename, char *data) {
   if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;

    lseek(disk_fd, 0, SEEK_SET);

    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            int datalen = strlen(data);

            lseek(disk_fd, fileoffset, SEEK_SET);
            write(disk_fd, data, datalen);

            // Metadata güncellemesi
            lseek(disk_fd, 0 - (sizeof(int) + sizeof(int)), SEEK_CUR);
            write(disk_fd, &datalen, sizeof(int));
            lseek(disk_fd, sizeof(int), SEEK_CUR);

            printf("fs_write: %s dosyasına %d byte yazıldı\n", filename, datalen);
            return 0;
        }
		printf("fs_write: Dosya bulunamadı: %s\n", filename);
    return -1;
}
}
int fs_read(char *filename) {
    if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;

    lseek(disk_fd, 0, SEEK_SET);

    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;
    char buffer[4096];

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            lseek(disk_fd, fileoffset, SEEK_SET);
            read(disk_fd, buffer, filesize);
            buffer[filesize] = '\0';
            printf("Dosya (%s) içeriği:\n%s\n", filename, buffer);
            return 0;
        }
    }

    printf("fs_read: Dosya bulunamadı: %s\n", filename);
    return -1;
}

int fs_delete(char *filename) {
   if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;

    lseek(disk_fd, 0, SEEK_SET);

    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            lseek(disk_fd, -RECORD_SIZE, SEEK_CUR);
            char empty[MAX_FILENAME_LEN] = {0};
            int zero = 0;
            write(disk_fd, empty, MAX_FILENAME_LEN);
            write(disk_fd, &zero, sizeof(int));
            write(disk_fd, &zero, sizeof(int));

            printf("fs_delete: %s dosyası silindi\n", filename);
            return 0;
        }
    }

    printf("fs_delete: Dosya bulunamadı: %s\n", filename);
    return -1;
}
int fs_ls(){
	if (check_disk() == -1) return -1;
    
	lseek(disk_fd, 0, SEEK_SET);
	
	
	char name_buf[MAX_FILENAME_LEN];
	int filesize;
	int fileoffset;
	int dosya_sayisi=0;
	
	printf("\n ---Dosya Listesi--- \n");
	
	for (int i=0; i<METADATA_SIZE/RECORD_SIZE; İ++){
		read(disk_fd,name_buf,MAX_FILENAME_LEN);
		read(disk_fd,&filesize,sizeof(int));
		read(disk_fd,&fileoffset,sizeof(int));
		
		if(name_buf[0] != '\0'){
			printf("%s (boyut: %d byte)\n",name_buf,filesize);
			dosya_sayisi++;
		}
}
if(dosya_sayisi==0){
	printf("Hiç dosya yok .\n");
}
printf("---------------\n");
return 0;
}
int fs_format(){
	if (disk_fd == -1) {
        perror("fs_format: Disk bağlı değil");
        return -1;
    }

    // Dosya boyutunu öğrenelim
    struct stat st;
    if (fstat(disk_fd, &st) == -1) {
        perror("fs_format: fstat hatası");
        return -1;
    }
    off_t disk_size = st.st_size;

    // Baştan sona tüm disk alanını sıfırlayacağız
    lseek(disk_fd, 0, SEEK_SET);

    char zero_buf[4096];
    memset(zero_buf, 0, sizeof(zero_buf));

    off_t yazilan = 0;
    while (yazilan < disk_size) {
        int n = write(disk_fd, zero_buf, sizeof(zero_buf));
        if (n <= 0) {
            perror("fs_format: Yazma hatası");
            return -1;
        }
        yazilan += n;
    }

    printf("fs_format: Disk tamamen sıfırlandı. (Metadata + Veri blokları)\n");
    return 0;
}
	int fs_rename(const char* oldname,const char*newname){
      if(disk_fd== -1){
       perror("fs_name:Disk bağlı değil");
      return -1;
}	
if (strlen(newname)>=MAX_FILENAME_LEN){
	printf("fs_rename:Yeni dosya adı çok uzun. \n");
	return -1;
}
lseek(disk_fd,0,SEEK_SET);
char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strcmp(name_buf, oldname) == 0) {
            // Eşleşme bulundu
            lseek(disk_fd, -RECORD_SIZE, SEEK_CUR);

            // Yeni ismi yaz
            char temp_buf[MAX_FILENAME_LEN];
            memset(temp_buf, 0, MAX_FILENAME_LEN); // Eski isim tam silinsin diye
            strncpy(temp_buf, newname, MAX_FILENAME_LEN - 1);
            write(disk_fd, temp_buf, MAX_FILENAME_LEN);

            printf("fs_rename: '%s' dosyası '%s' olarak değiştirildi.\n", oldname, newname);
            return 0;
        }
    }

    printf("fs_rename: '%s' adlı dosya bulunamadı.\n", oldname);
    return -1;
}
int fs_exists(const char* filename){
	if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;
	lseek(disk_fd,0,SEEK_SET);
	
	char name_buf[MAX_FILENAME_LEN];
	int filesize;
	int fileoffset;
	for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            return 1; // Dosya bulundu
        }
    }

    return 0; // Dosya bulunamadı
}
int fs_size(char*filename){
	if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;
	lseek(disk_fd, 0,SEEK_SET);
	
	 char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            printf("%s dosyasının boyutu: ", filename);
            if (filesize >= (1024 * 1024)) {
                printf("%.2f MB\n", filesize / (1024.0 * 1024.0));
            } else if (filesize >= 1024) {
                printf("%.2f KB\n", filesize / 1024.0);
            } else {
                printf("%d byte\n", filesize);
            }
            return filesize;
        }
    }

    printf("fs_size: Dosya bulunamadı: %s\n", filename);
    return -1;
}
int fs_append(char *filename,char *data,int size){
	if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;

    lseek(disk_fd, 0, SEEK_SET);

    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            // Dosya bulundu

            // Eski verinin sonuna eklemek için konumla
            lseek(disk_fd, fileoffset + filesize, SEEK_SET);

            // Veriyi yaz
            write(disk_fd, data, size);

            // Metadata'daki boyutu güncelle
            filesize += size;
            lseek(disk_fd, - (size + sizeof(int)), SEEK_CUR); // Geri gelip sadece boyutu güncelle
            lseek(disk_fd, MAX_FILENAME_LEN, SEEK_CUR); // İsim alanını atla
            write(disk_fd, &filesize, sizeof(int));

            printf("fs_append: %s dosyasına %d byte eklendi\n", filename, size);
            return 0;
        }
    }

    printf("fs_append: Dosya bulunamadı: %s\n", filename);
    return -1;
}
int fs_truncate(char *filename,int newsize){
	if (check_disk() == -1) return -1;
    if (check_filename(filename) == -1) return -1;
	 if (newsize < 0) {
        printf("fs_truncate: Negatif boyut geçersiz.\n");
        return -1;
    }

    lseek(disk_fd, 0, SEEK_SET);
    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strcmp(name_buf, filename) == 0) {
            if (newsize > filesize) {
                printf("fs_truncate: Yeni boyut eski boyuttan büyük olamaz.\n");
                return -1;
            }

            // Dosya içeriğini güncelle (gerekirse sıfırla)
            char zero = 0;
            lseek(disk_fd, fileoffset + newsize, SEEK_SET);
            for (int i = newsize; i < filesize; i++) {
                write(disk_fd, &zero, 1);
            }

            // Metadata'yı güncelle
            lseek(disk_fd, -RECORD_SIZE + MAX_FILENAME_LEN, SEEK_CUR); // geri dön
            write(disk_fd, &newsize, sizeof(int)); // yeni boyut

            printf("fs_truncate: Dosya '%s' %d byte'a kısaltıldı.\n", filename, newsize);
            return 0;
        }
    }

    printf("fs_truncate: Dosya bulunamadı: %s\n", filename);
    return -1;
}
int fs_copy(char*source,char*dest){
	if (check_disk() == -1) return -1;
	  if (strlen(dest) >= MAX_FILENAME_LEN) {
        printf("fs_copy: Hedef dosya adı çok uzun.\n");
        return -1;
    }

    lseek(disk_fd, 0, SEEK_SET);
    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    // 1. Kaynak dosyayı bul
    int found = 0;
    int source_size = 0;
    int source_offset = 0;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strcmp(name_buf, source) == 0) {
            found = 1;
            source_size = filesize;
            source_offset = fileoffset;
            break;
        }
    }

    if (!found) {
        printf("fs_copy: Kaynak dosya bulunamadı: %s\n", source);
        return -1;
    }

    // 2. Kaynak veriyi oku
    char *buffer = malloc(source_size);
    lseek(disk_fd, source_offset, SEEK_SET);
    read(disk_fd, buffer, source_size);

    // 3. Hedef dosya var mı kontrol et
    lseek(disk_fd, 0, SEEK_SET);
    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strcmp(name_buf, dest) == 0) {
            printf("fs_copy: Hedef dosya zaten var: %s\n", dest);
            free(buffer);
            return -1;
        }
    }

    // 4. Boş kayıt bul ve yeni dosyayı yaz
    lseek(disk_fd, 0, SEEK_SET);
    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (name_buf[0] == '\0') {
            lseek(disk_fd, -RECORD_SIZE, SEEK_CUR);
            write(disk_fd, dest, MAX_FILENAME_LEN);
            int new_offset = METADATA_SIZE + (i * 4096);
            write(disk_fd, &source_size, sizeof(int));
            write(disk_fd, &new_offset, sizeof(int));

            // İçeriği yaz
            lseek(disk_fd, new_offset, SEEK_SET);
            write(disk_fd, buffer, source_size);
            printf("fs_copy: %s -> %s kopyalandı (%d byte)\n", source, dest, source_size);
            free(buffer);
            return 0;
        }
    }

    printf("fs_copy: Yeni dosya için yer kalmadı\n");
    free(buffer);
    return -1;
}
int fs_mv(char *old_path, char *new_path) {
 if (check_disk() == -1) return -1;
	  if (strlen(new_path) >= MAX_FILENAME_LEN) {
        printf("fs_mv: Hedef dosya adı çok uzun.\n");
        return -1;
    }
	lseek(disk_fd, 0, SEEK_SET);
    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    // Önce yeni ad zaten kullanılıyor mu?
    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));
        if (strncmp(name_buf, new_path, MAX_FILENAME_LEN) == 0) {
            printf("fs_mv: '%s' zaten mevcut.\n", new_path);
            return -1;
        }
    }

    // Eski adı bul ve değiştir
    lseek(disk_fd, 0, SEEK_SET);
    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        off_t pos = lseek(disk_fd, 0, SEEK_CUR);
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, old_path, MAX_FILENAME_LEN) == 0) {
            lseek(disk_fd, pos, SEEK_SET);
            char newname_buf[MAX_FILENAME_LEN] = {0};
            strncpy(newname_buf, new_path, MAX_FILENAME_LEN - 1);
            write(disk_fd, newname_buf, MAX_FILENAME_LEN);
            printf("fs_mv: '%s' dosyası '%s' olarak yeniden adlandırıldı.\n", old_path, new_path);
            return 0;
        }
    }

    printf("fs_mv: '%s' adlı dosya bulunamadı.\n", old_path);
    return -1;
}
int fs_defragment(){
 if (check_disk() == -1) return -1;
 lseek(disk_fd, 0, SEEK_SET);

    char name_buf[MAX_FILENAME_LEN];
    int filesize, fileoffset;

    int new_offset = METADATA_SIZE; // İlk boş blok buradan başlar

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        off_t record_pos = i * RECORD_SIZE;
        lseek(disk_fd, record_pos, SEEK_SET);
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (name_buf[0] != '\0' && filesize > 0) {
            // Dosyayı oku
            char *buffer = malloc(filesize);
            lseek(disk_fd, fileoffset, SEEK_SET);
            read(disk_fd, buffer, filesize);

            // Yeni konuma yaz
            lseek(disk_fd, new_offset, SEEK_SET);
            write(disk_fd, buffer, filesize);
            free(buffer);

            // Metadata'daki offset’i güncelle
            lseek(disk_fd, record_pos + MAX_FILENAME_LEN + sizeof(int), SEEK_SET);
            write(disk_fd, &new_offset, sizeof(int));

            // Yeni offset’i ayarla
            new_offset += filesize;
        }
    }

    printf("fs_defragment: Tüm dosyalar yeniden hizalandı.\n");
    return 0;
}
int fs_check_integrity(){
	if (check_disk() == -1) return -1;
	 lseek(disk_fd, 0, SEEK_SET);
    char name_buf[MAX_FILENAME_LEN];
    int filesize, fileoffset;
    int is_valid = 1;

    struct stat st;
    if (fstat(disk_fd, &st) == -1) {
        perror("fs_check_integrity: Disk boyutu alınamadı");
        return -1;
    }

    off_t disk_size = st.st_size;

    printf("Dosya sistemi bütünlük kontrolü:\n");

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (name_buf[0] != '\0') {
            if (fileoffset + filesize > disk_size) {
                printf("HATA: %s dosyasının boyutu veya konumu hatalı.\n", name_buf);
                is_valid = 0;
            }

            // Diğer dosyalarla çakışma kontrolü istenirse buraya eklenebilir.
        }
    }

    if (is_valid) {
        printf("Tüm dosyalar geçerli ve uyumlu.\n");
        return 0;
    } else {
        printf("Tutarsızlıklar tespit edildi.\n");
        return -1;
    }
}
int fs_backup(const char *backup_path) {
    if (check_disk() == -1) return -1;

    int backup_fd = open(backup_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (backup_fd == -1) {
        perror("fs_backup: Yedek dosyası oluşturulamadı");
        return -1;
    }

    // Disk boyutunu al
    struct stat st;
    if (fstat(disk_fd, &st) == -1) {
        perror("fs_backup: fstat hatası");
        close(backup_fd);
        return -1;
    }

    off_t disk_size = st.st_size;
    lseek(disk_fd, 0, SEEK_SET);

    char buffer[4096];
    off_t remaining = disk_size;

    while (remaining > 0) {
        ssize_t to_read = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
        ssize_t n = read(disk_fd, buffer, to_read);
        if (n <= 0) {
            perror("fs_backup: okuma hatası");
            close(backup_fd);
            return -1;
        }

        write(backup_fd, buffer, n);
        remaining -= n;
    }

    close(backup_fd);
    printf("fs_backup: Disk başarıyla '%s' dosyasına yedeklendi.\n", backup_path);
    return 0;
}
int fs_cat(const char*filename){
	if (check_disk() == -1) return -1;
	lseek(disk_fd, 0, SEEK_SET);
    char name_buf[MAX_FILENAME_LEN];
    int filesize;
    int fileoffset;

    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &fileoffset, sizeof(int));

        if (strncmp(name_buf, filename, MAX_FILENAME_LEN) == 0) {
            char *buffer = malloc(filesize + 1);
            lseek(disk_fd, fileoffset, SEEK_SET);
            read(disk_fd, buffer, filesize);
            buffer[filesize] = '\0';

            printf("%s\n", buffer);
            free(buffer);
            return 0;
        }
    }

    printf("fs_cat: Dosya bulunamadı: %s\n", filename);
    return -1;
}
int fs_restore(const char *backup_path) {
    if (disk_fd != -1) {
        close(disk_fd); // Açık diski kapat
    }

    // Yedeği orijinal disk dosyasına geri yaz
    FILE *backup_file = fopen(backup_path, "rb");
    FILE *disk_file = fopen("disk.img", "wb");

    if (!backup_file || !disk_file) {
        perror("fs_restore: Dosya açma hatası");
        if (backup_file) fclose(backup_file);
        if (disk_file) fclose(disk_file);
        return -1;
    }

    char buffer[4096];
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), backup_file)) > 0) {
        fwrite(buffer, 1, bytes, disk_file);
    }

    fclose(backup_file);
    fclose(disk_file);

    // Yeniden aç
    disk_fd = open("disk.img", O_RDWR);
    if (disk_fd == -1) {
        perror("fs_restore: disk.img yeniden açılamadı");
        return -1;
    }

    printf("fs_restore: Yedekten geri yükleme başarılı\n");
    return 0;
}
int fs_diff(const char *file1, const char *file2) {
    if (check_disk() == -1) return -1;

    lseek(disk_fd, 0, SEEK_SET);

    char name_buf[MAX_FILENAME_LEN];
    int filesize1 = -1, offset1 = -1;
    int filesize2 = -1, offset2 = -1;

    // 1. ve 2. dosyaların metadata'sını bul
    for (int i = 0; i < METADATA_SIZE / RECORD_SIZE; i++) {
        read(disk_fd, name_buf, MAX_FILENAME_LEN);
        int filesize, offset;
        read(disk_fd, &filesize, sizeof(int));
        read(disk_fd, &offset, sizeof(int));

        if (strncmp(name_buf, file1, MAX_FILENAME_LEN) == 0) {
            filesize1 = filesize;
            offset1 = offset;
        }
        if (strncmp(name_buf, file2, MAX_FILENAME_LEN) == 0) {
            filesize2 = filesize;
            offset2 = offset;
        }
    }

    if (filesize1 == -1 || filesize2 == -1) {
        printf("fs_diff: Dosyalardan biri ya da ikisi bulunamadı.\n");
        return -1;
    }

    if (filesize1 != filesize2) {
        printf("fs_diff: Dosyalar farklı (boyutları farklı).\n");
        return 1;
    }

    // İçerikleri karşılaştır
    char *buf1 = malloc(filesize1);
    char *buf2 = malloc(filesize2);

    lseek(disk_fd, offset1, SEEK_SET);
    read(disk_fd, buf1, filesize1);

    lseek(disk_fd, offset2, SEEK_SET);
    read(disk_fd, buf2, filesize2);

    int result = memcmp(buf1, buf2, filesize1);
    free(buf1);
    free(buf2);

    if (result == 0) {
        printf("fs_diff: Dosyalar aynı.\n");
        return 0;
    } else {
        printf("fs_diff: Dosyalar farklı (içerikleri farklı).\n");
        return 1;
    }
}


void fs_log(const char *operation, const char *filename, const char *result) {
    FILE *log_file = fopen("fs_log.txt", "a");
    if (!log_file) {
        perror("fs_log: Log dosyası açılamadı");
        return;
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strcspn(timestamp, "\n")] = 0; // new line kaldır

    fprintf(log_file, "[%s] %s - %s: %s\n", timestamp, operation, filename, result);
    fclose(log_file);
}
