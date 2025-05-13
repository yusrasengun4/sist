#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

int main() {
    if (fs_init() == -1) {
        return 1;
    }

    int choice;
    char dosya_adi[MAX_FILENAME_LEN];
    char dosya_adi2[MAX_FILENAME_LEN];
    char veri[4096];
    char newname[MAX_FILENAME_LEN];
    char path[128];
    char backup_name[100];

    do {
        printf("\n=== Dosya Sistemi İşlemleri ===\n");
        printf("1. Dosya Oluştur\n");
        printf("2. Dosya Sil\n");
        printf("3. Dosyaya yaz\n");
        printf("4. Dosyadan oku\n");
        printf("5. Dosyaları listele\n");
        printf("6. Format at\n");
        printf("7. Dosyayı yeniden adlandır\n");
        printf("8. Dosyayı bul\n");
        printf("9. Dosya boyutu\n");
        printf("10. Dosyanın sonuna veri ekle\n");
        printf("11. Dosyayı küçült\n");
        printf("12. Dosyayı kopyala\n");
        printf("13. Dosyayı taşı\n");
        printf("14. Boş alanları birleştir (defragment)\n");
        printf("15. Bütünlüğü kontrol et\n");
        printf("16. Diski yedekle\n");
        printf("17. Dosya içeriğini yazdır\n");
        printf("18. Yedekten Geri Yükle\n");
        printf("19. İki Dosyayı Karşılaştır\n");
        printf("0. Çıkış\n");
        printf("Seçiminiz: ");
        scanf("%d", &choice);
        getchar(); // newline temizliği

        switch (choice) {
            case 1:
                printf("Dosya adını girin: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                fs_create(dosya_adi);
                break;

            case 2:
                printf("Silinecek dosya adını girin: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                fs_delete(dosya_adi);
                break;

            case 3:
                printf("Dosya adı: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                printf("Yazılacak veriyi girin: ");
                fgets(veri, sizeof(veri), stdin);
                veri[strcspn(veri, "\n")] = 0;
                fs_write(dosya_adi, veri);
                break;

            case 4:
                printf("Dosya adı: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                fs_read(dosya_adi);
                break;

            case 5:
                fs_ls();
                break;

            case 6:
                fs_format();
                break;

            case 7:
                printf("Eski dosya adını girin: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                printf("Yeni dosya adını girin: ");
                fgets(newname, sizeof(newname), stdin);
                newname[strcspn(newname, "\n")] = 0;
                fs_rename(dosya_adi, newname);
                break;

            case 8:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                getchar();
                if (fs_exists(dosya_adi) == 1) {
                    printf("Dosya VAR: %s\n", dosya_adi);
                } else {
                    printf("Dosya YOK: %s\n", dosya_adi);
                }
                break;

            case 9:
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                getchar();
                fs_size(dosya_adi);
                break;

            case 10:
                printf("Dosya adı: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                printf("Eklenecek veriyi girin: ");
                fgets(veri, sizeof(veri), stdin);
                veri[strcspn(veri, "\n")] = 0;
                if (fs_append(dosya_adi, veri, strlen(veri)) == 0) {
                    printf("Veri başarıyla eklendi.\n");
                } else {
                    printf("Veri eklenemedi.\n");
                }
                break;

            case 11: {
                int newsize;
                printf("Dosya adı: ");
                scanf("%s", dosya_adi);
                printf("Yeni boyut (byte): ");
                scanf("%d", &newsize);
                getchar();
                fs_truncate(dosya_adi, newsize);
                break;
            }

            case 12:
                printf("Kaynak dosya: ");
                scanf("%s", dosya_adi);
                printf("Hedef dosya: ");
                scanf("%s", dosya_adi2);
                getchar();
                fs_copy(dosya_adi, dosya_adi2);
                break;

            case 13:
                printf("Eski dosya adı: ");
                scanf("%s", dosya_adi);
                printf("Yeni dosya adı: ");
                scanf("%s", dosya_adi2);
                getchar(); // stdin temizliği (iyi)

               if (fs_mv(dosya_adi, dosya_adi2) == 0) {
               printf("Dosya başarıyla yeniden adlandırıldı.\n");
             } else {
           printf("Dosya yeniden adlandırılamadı.\n");
           }
            break;

            case 14:
                if (fs_defragment() == 0) {
                    printf("Disk başarıyla birleştirildi.\n");
                } else {
                    printf("Birleştirme başarısız.\n");
                }
                break;

            case 15:
                fs_check_integrity();
                break;

            case 16:
                printf("Yedek dosya adı: ");
                scanf("%s", backup_name);
                getchar();
                fs_backup(backup_name);
                break;

            case 17:
                printf("Dosya adı: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                fs_cat(dosya_adi);
                break;

            case 18:
                printf("Yedek dosya yolu: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = 0;
                fs_restore(path);
                break;

            case 19:
                printf("1. dosya adı: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                printf("2. dosya adı: ");
                fgets(dosya_adi2, sizeof(dosya_adi2), stdin);
                dosya_adi2[strcspn(dosya_adi2, "\n")] = 0;
                fs_diff(dosya_adi, dosya_adi2);
                break;

            case 0:
                printf("Çıkılıyor...\n");
                break;

            default:
                printf("Geçersiz seçim.\n");
                break;
        }

    } while (choice != 0);

    fs_close();
    return 0;
}
