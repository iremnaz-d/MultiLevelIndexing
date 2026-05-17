#include<stdio.h>
#include<json-c/json.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define HEAP_MAX 4
int allCity_number = 0;
int allProduct_number = 0;


#pragma region Structs
typedef struct _Product_Info{ //product object
    char name[40];
    char brand[40];
    char category[32];
  }Product_Info;

  typedef struct _Pricing{ //product object
    int price;
    char currency[4];
  }Pricing;
  
  typedef struct _Inventory{ //product object
    int stock;
    char warehouse[4];
  }Inventory;

  typedef struct _Product{
    char product_id[64];
    Product_Info product_info;
    Pricing pricing;
    Inventory inventory;
    char isbn[32];
    char description[64];
    char extra[16];
    long dat_offset;
  }Product;

  typedef struct _Meta{ //city object
      int population;
      char region[32];
  }Meta;

  typedef struct _City{
    char city_name[16];
    Meta meta;
    Product products[12];
    int product_number;
  }City;

  typedef struct _Country{
    char country[16];
    char country_code[4];
    City cities[32];
    int city_number;
    int offset_CityIndex;
  }Country;

  typedef struct { //City_Index
    int self_index;
    City city;
    int next_city_index;
    int product_index;
  }City_Index;

  typedef struct { //Product_Index
    int self_index;
    Product product;
    int product_offset;
    long dat_offset;
  } Product_Index;
  #pragma endregion

#pragma region Search Operations 

  typedef struct{ //SearchCity için
    City city;
    Country country;
  }FoundCity;

  typedef struct{ //SearchCountry için
    Country country;
    City city;
    Product product;
  }FoundProduct;


   Country* BinarySearch_Country(Country countries[], int country_number, char countryName[]){
    int middle = country_number/2; int end = country_number-1; int start = 0;
    while(start <= end){
      Country *compareCountry = &countries[middle];
      if(strcasecmp(countryName, compareCountry->country) > 0){ //arattığımız country daha büyükse sağa geçer
        start = middle+1; middle = (end+start)/2;
      }
      else if(strcasecmp(countryName, compareCountry->country) < 0){ //arattığımız country daha küçükse sola geçer
        end = middle-1; middle = (end+start)/2;
      }
      else{ //countryi bulduysak
        return &countries[middle];
      }
    }
    return NULL;
  }

  FoundCity* Search_City(Country countries[], int country_number, char cityName[], City_Index city_index[]){

    FoundCity* foundcity = malloc(sizeof(FoundCity)); //yine malloc olmayınca segmentation fault aldım
    for (int i = 0; i < country_number; i++) {
        int offset = countries[i].offset_CityIndex;
        
        while (offset != -1) {
            if (strcasecmp(cityName, city_index[offset].city.city_name) == 0) {
              foundcity->city = city_index[offset].city; //bulunan şehri kaydet
              foundcity->country = countries[i]; //bulunan şehrin ülkesini kaydet
              return foundcity;
            }
            offset = city_index[offset].next_city_index;
        }
    }
    
    strcpy(foundcity->city.city_name, "City not found."); //city bulunmadıysa
    return foundcity;
  }

  FoundProduct* Search_Product(Country countries[], int country_number, char productName[], City_Index city_index[], Product_Index product_index[]){
    FoundProduct* foundproduct = malloc(sizeof(FoundProduct));
    for (int i = 0; i < country_number; i++) {
        int offset = countries[i].offset_CityIndex;
        
        while (offset != -1) {
            int p_offset = city_index[offset].product_index;
            
            while (p_offset != -1) {
                if (strcasecmp(productName, product_index[p_offset].product.product_info.name) == 0) {
                  foundproduct->product = product_index[p_offset].product; //bulunan productı kaydet
                  foundproduct->city = city_index[offset].city; //bulunan productın citysini kaydet
                  foundproduct->country = countries[i]; //bulunan productın ülkesini kaydet
                    return foundproduct;
                }
                p_offset = product_index[p_offset].product_offset; //diğer ürüne geçiyo
            }
            offset = city_index[offset].next_city_index; //diğer şehre geçiyo
        }
    }
    
    
    strcpy(foundproduct->product.product_info.name, "Product not found.");
    return foundproduct;
  }


  #pragma endregion

#pragma region Heap

  typedef struct{
    City data;
    int run_number;
  } CityNode;

  typedef struct{
    Product data;
    int run_number;
  } ProductNode;

  typedef struct {
    Country data;
    int run_number;
} CountryNode;

void heapify_country(CountryNode heap[], int i, int size) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < size) {
        if (heap[left].run_number < heap[smallest].run_number) smallest = left;
        else if (heap[left].run_number == heap[smallest].run_number && 
                 strcasecmp(heap[left].data.country, heap[smallest].data.country) < 0) smallest = left;
    }
    if (right < size) {
        if (heap[right].run_number < heap[smallest].run_number) smallest = right;
        else if (heap[right].run_number == heap[smallest].run_number && 
                 strcasecmp(heap[right].data.country, heap[smallest].data.country) < 0) smallest = right;
    }

    if (smallest != i) {
        CountryNode temp = heap[i];
        heap[i] = heap[smallest];
        heap[smallest] = temp;
        heapify_country(heap, smallest, size);
    }
}

  void heapify_city(CityNode heap[], int rootIndex, int heapSize) { //roota konan node'u doğru yere kaydırır
    int smallest = rootIndex;
    int leftChild = 2 * rootIndex + 1;
    int rightChild = 2 * rootIndex + 2;

    if (leftChild < heapSize) {
        if (heap[leftChild].run_number < heap[smallest].run_number || 
           (heap[leftChild].run_number == heap[smallest].run_number && strcasecmp(heap[leftChild].data.city_name, heap[smallest].data.city_name) < 0)) {
            smallest = leftChild; //leftChildın run_numberı küçükse veya run_numberlar eşitse ve left_child küçükse
        }
    }

    if (rightChild < heapSize) {
        if (heap[rightChild].run_number < heap[smallest].run_number || 
           (heap[rightChild].run_number == heap[smallest].run_number && strcasecmp(heap[rightChild].data.city_name, heap[smallest].data.city_name) < 0)) {
            smallest = rightChild;
        }
    }

    if (smallest != rootIndex) {//root en küçük değilse root ile smallestı yer değiştir
        CityNode temp = heap[rootIndex];
        heap[rootIndex] = heap[smallest];
        heap[smallest] = temp;
        heapify_city(heap, smallest, heapSize);
    }
}


void heapify_product(ProductNode heap[], int rootIndex, int heapSize) {
    int smallest = rootIndex;
    int leftChild = 2 * rootIndex + 1;
    int rightChild = 2 * rootIndex + 2;

    if (leftChild < heapSize) {
        if (heap[leftChild].run_number < heap[smallest].run_number || 
           (heap[leftChild].run_number == heap[smallest].run_number && strcasecmp(heap[leftChild].data.product_info.name, heap[smallest].data.product_info.name) < 0)) {
            smallest = leftChild;
        }
    }

    if (rightChild < heapSize) {
        if (heap[rightChild].run_number < heap[smallest].run_number || 
           (heap[rightChild].run_number == heap[smallest].run_number && strcasecmp(heap[rightChild].data.product_info.name, heap[smallest].data.product_info.name) < 0)) {
            smallest = rightChild;
        }
    }

    if (smallest != rootIndex) {
        ProductNode temp = heap[rootIndex];
        heap[rootIndex] = heap[smallest];
        heap[smallest] = temp;
        heapify_product(heap, smallest, heapSize);
    }
}


void insertCityToHeap(CityNode heap[], CityNode insertNode, int heapSize) { //roota ekliyoruz sadece
    heap[0] = insertNode;
    heapify_city(heap, 0, heapSize);
}

void insertProductToHeap(ProductNode heap[], ProductNode insertNode, int heapSize) { 
    heap[0] = insertNode;
    heapify_product(heap, 0, heapSize);
}


 #pragma endregion

#pragma region Prints
void printProductWithHeading(Product *p){
  printf("%-16s %-16s %-10s %-16s %-16s %-16s %-20s %-20s %s\n", "NAME", "BRAND", "PRICING", "CATEGORY", "ID", "INVENTORY", "ISBN", "DESCRIPTION", "EXTRA");
  
  printf("%-16s %-16s %-5d %-4s %-16s %-16s %-9s %-6d %-20s %-20s %s\n",
    p->product_info.name, p->product_info.brand, p->pricing.price, p->pricing.currency,p->product_info.category,p->product_id,p->inventory.warehouse,p->inventory.stock,p->isbn,p->description,p->extra);
 }

 void printProduct(Product p){
  printf("%-16s %-16s %-5d %-5s %-16s %-16s %-6d %-6s %-20s %-10s",
    p.product_info.name,p.product_info.brand,p.pricing.price,p.pricing.currency,p.product_info.category,p.product_id,p.inventory.warehouse,p.inventory.stock,p.isbn,p.description,p.extra);
 }

 void printAllProducts(City city){
  printf("%-16s %-16s %-10s %-16s %-16s %-16s %-20s %-10s %s\n","NAME", "BRAND", "PRICING", "CATEGORY", "ID", "INVENTORY", "ISBN", "DESCRIPTION", "EXTRA");
  for (int i = 0; i < city.product_number; i++){
    printProduct(city.products[i]);
  }
 }

 void printRSS(Country input[], Country output[], CountryNode heap[],int inputIndex, int outputIndex, int country_number,int heapSize){
    int input_remaining = country_number - inputIndex;
    
    // Üç sütundan en uzun olanı buluyoruz ki tablo yarım kesilmesin
    int max_rows = outputIndex;
    if (heapSize > max_rows) max_rows = heapSize;
    if (input_remaining > max_rows) max_rows = input_remaining;

    printf("\n%-25s | %-30s | %-25s\n", "OUTPUT", "HEAP", "INPUT");
    printf("--------------------------|--------------------------------|--------------------------\n");

    for (int i = 0; i < max_rows; i++) {
        char out_str[50] = "";
        char heap_str[50] = "";
        char in_str[50] = "";

        // Sol Sütun: Output (Artık sıralanmış ve Run'lara ayrılmış kısım)
        if (i < outputIndex) {
            sprintf(out_str, "%s", output[i].country);
        }

        // Orta Sütun: Heap (İçindeki elemanlar ve Run numaraları)
        if (i < heapSize) {
            sprintf(heap_str, "[R%d] %s", heap[i].run_number, heap[i].data.country);
        }

        // Sağ Sütun: Input (Henüz okunmamış, bekleyen elemanlar)
        if (i < input_remaining) {
            sprintf(in_str, "%s", input[inputIndex + i].country);
        }

        // Satırı ekrana bas
        printf("%-25s | %-30s | %-25s\n", out_str, heap_str, in_str);
    }
    printf("--------------------------|--------------------------------|--------------------------\n\n");
    getchar();
}
 #pragma endregion

#pragma region Sort

void Visualized_RSS_Countries(Country unsorted[], int country_number, Country output[]) {
    if (country_number == 0) return;
    CountryNode heap[HEAP_MAX];
    int inputIndex = 0, outputIndex = 0;
    int current_print_run = 1;

    int heapSize;
    if (country_number < HEAP_MAX) { //full HEAP_MAX almamak için tam uzunluğu bilmemiz gerekiyo, display etcez
        heapSize = country_number;
    } else {
        heapSize = HEAP_MAX;
    }

    for (int i = 0; i < heapSize; i++) { 
        heap[i].data = unsorted[inputIndex++];
        heap[i].run_number = 1;
    }
    for (int i = (heapSize / 2) - 1; i >= 0; i--) {
        heapify_country(heap, i, heapSize);
    }


    while (inputIndex < country_number) {
        CountryNode minNode = heap[0];
        output[outputIndex++] = minNode.data;
    

        printf("%s --> OUTPUT         ", minNode.data.country);

        CountryNode newNode;
        newNode.data = unsorted[inputIndex++];
        if (strcasecmp(newNode.data.country, minNode.data.country) >= 0) {
            newNode.run_number = minNode.run_number;
        } else {
            newNode.run_number = minNode.run_number + 1;
        }

        heap[0] = newNode;
        heapify_country(heap, 0, heapSize);
        printf("%s --> HEAP",newNode.data.country);
        printRSS(unsorted,output,heap,inputIndex,outputIndex,country_number,heapSize);
    }

    int size = heapSize;
    while (size > 0) {
        CountryNode minNode = heap[0];
        output[outputIndex++] = minNode.data;
        
        if (minNode.run_number > current_print_run) {
            current_print_run = minNode.run_number;
        }

        heap[0] = heap[size - 1];
        size--;
        if (size > 0) heapify_country(heap, 0, size);
    }
    printf("\n");
}

void Visualized_Merge_Countries(Country array[], int num_elements) {
    if (num_elements <= 1) return;
    Country *temp = (Country *)malloc(num_elements * sizeof(Country));
    bool sorted = false;
    int pass_number = 1;

    while (!sorted) {
        sorted = true; int i = 0; int merge_count = 0;

        while (i < num_elements) {
            int start1 = i;
            printf("\nRUN-1: ");
            while (i + 1 < num_elements && strcasecmp(array[i].country, array[i+1].country) <= 0){printf("%s ", array[i].country); i++;}  //run1in sonunu buluyo ve run1 print
            int end1 = i; printf("%s ", array[i].country);
            i++;
            

            if (i >= num_elements) break; //daha eleman yoksa yani run2 yoksa bitir

            sorted = false; merge_count++;
            int start2 = i;
            printf("\nRUN-2: ");
            while (i + 1 < num_elements && strcasecmp(array[i].country, array[i+1].country) <= 0) {printf("%s ", array[i].country); i++;} //run2nin sonunu buluyo ve run2 print
            int end2 = i; printf("%s ", array[i].country);
            i++;
            getchar();

            int p1 = start1, p2 = start2, k = start1;
            while (p1 <= end1 && p2 <= end2) {
                if (strcasecmp(array[p1].country, array[p2].country) <= 0){
                    temp[k] = array[p1];
                    printf("\n%s < %s\n", array[p1].country, array[p2].country);
                    k++; p1++;
                }     
                else{
                    temp[k] = array[p2];
                    printf("\n%s > %s\n", array[p1].country, array[p2].country);
                    p2++; k++;
                } 
                printf("TEMP: ");
                for (int m = 0; m < k; m++){ //tempi print ediyo
                    printf("%s ", temp[m].country);
                }
                getchar();
            }
            while (p1 <= end1) temp[k++] = array[p1++];
            while (p2 <= end2) temp[k++] = array[p2++];

            for (int j = start1; j <= end2; j++) array[j] = temp[j];
        }
        if (!sorted) {
            printf("\nMerge Pass %d completed.\n", pass_number++);
        }
    }
    printf("\n\nMerge is complete..\n\n");
    free(temp);
}

void RSS_City(City unsorted[], int city_number, City output[]) {
    if (city_number == 0) return;
    CityNode heap[HEAP_MAX];
    int inputIndex = 0, outputIndex = 0;

    for (int i = 0; i < HEAP_MAX; i++) { //önce heapi tamamen dolduruyoz
        heap[i].data = unsorted[inputIndex++];
        heap[i].run_number = 1;
    }

    for (int i = (HEAP_MAX / 2) - 1; i >= 0; i--) {
        heapify_city(heap, i, HEAP_MAX);
    }

    while (inputIndex < city_number) {
        CityNode minNode = heap[0]; //roottaki eleman outputa atılıyor
        output[outputIndex++] = minNode.data;

        CityNode newNode;
        newNode.data = unsorted[inputIndex++];
        if (strcasecmp(newNode.data.city_name, minNode.data.city_name) >= 0) { //input > output
         newNode.run_number = minNode.run_number;
        } 
        else {
         newNode.run_number = minNode.run_number + 1; //input<output, newNode diğer runa atılır
        }

        insertCityToHeap(heap,newNode,HEAP_MAX); //input insert edilir
    }

    int size = HEAP_MAX;
    while (size > 0) { //heapte kalanlar normal heap boşaltır gibi boşaltılır
        output[outputIndex++] = heap[0].data; //root çıkarılır
        heap[0] = heap[size - 1]; //son eleman roota alınır
        size--;
        if (size > 0) {
            heapify_city(heap, 0, size);
        }
    }
}

void RSS_Product(Product unsorted[], int product_number, Product sorted_output[]) {
    if (product_number == 0) return;
    ProductNode heap[HEAP_MAX];
    int inputIndex = 0, outputIndex = 0;

    for (int i = 0; i < HEAP_MAX; i++) {
        heap[i].data = unsorted[inputIndex++];
        heap[i].run_number = 1;
    }

    for (int i = (HEAP_MAX / 2) - 1; i >= 0; i--) {
        heapify_product(heap, i, HEAP_MAX);
    }

    while (inputIndex < product_number) {
        ProductNode minNode = heap[0];
        sorted_output[outputIndex++] = minNode.data;

        ProductNode newNode;
        newNode.data = unsorted[inputIndex++];
        if (strcasecmp(newNode.data.product_info.name, minNode.data.product_info.name) >= 0) {
         newNode.run_number = minNode.run_number;
        } 
        else {
         newNode.run_number = minNode.run_number + 1;
        }

        insertProductToHeap(heap,newNode,HEAP_MAX);
    }

    int size = HEAP_MAX;
    while (size > 0) {
        sorted_output[outputIndex++] = heap[0].data;
        heap[0] = heap[size - 1];
        size--;
        if (size > 0) {
            heapify_product(heap, 0, size);
        }
    }
}

void Merge_Runs_Cities(City array[], int num_elements) {
    if (num_elements <= 1) return;
    City *temp = (City *)malloc(num_elements * sizeof(City));
    bool sorted = false;

    while (!sorted) {
        int i = 0;  sorted = true; // Başlangıçta sıralı varsayıyoruz

        while (i < num_elements) {
            int start1 = i; //run1 başlangıcı
            while (i + 1 < num_elements && strcasecmp(array[i].city_name, array[i+1].city_name) <= 0) {//1. Runın sonunu buluyo
                i++;
            }
            int end1 = i; //run1 sonu
            i++; 

            if (i >= num_elements) break; // başka run yoksa

            sorted = false; 
            int start2 = i;
            while (i + 1 < num_elements && strcasecmp(array[i].city_name, array[i+1].city_name) <= 0) {//Run2nin sonunu buluyo
                i++;
            }
            int end2 = i;
            i++;

            int p1 = start1, p2 = start2, k = start1;
            while (p1 <= end1 && p2 <= end2) {
                if (strcasecmp(array[p1].city_name, array[p2].city_name) <= 0) { //run1 < run2 ise run1deki tempe atılır, pointer ilerletilir
                    temp[k] = array[p1]; k++; p1++;
                } else { //run1 > run2 ise run2'deki tempe atılır
                    temp[k] = array[p2]; k++; p2++;
                }
            }
            while (p1 <= end1){ temp[k] = array[p1]; k++; p1++;} //en son runlarda kalan elemanları da atmak gerekiyomuş. evet eleman kalıyomuş
            while (p2 <= end2){ temp[k] = array[p2]; k++; p2++;}

            for (int j = start1; j <= end2; j++) { //tempi geri arraye atıyo
                array[j] = temp[j];
            }
        }
    }
    free(temp); // MALLOC YAPMIŞTIM
}

void Merge_Runs_Products(Product array[], int num_elements) {
    if (num_elements <= 1) return;
    Product *temp = (Product *)malloc(num_elements * sizeof(Product));
    bool sorted = false;

    while (!sorted) {
        sorted = true; int i = 0;

        while (i < num_elements) {
            int start1 = i;
            while (i + 1 < num_elements && strcasecmp(array[i].product_info.name, array[i+1].product_info.name) <= 0) {
                i++;
            }
            int end1 = i;
            i++;

            if (i >= num_elements) break;

            sorted = false;
            int start2 = i;
            while (i + 1 < num_elements && strcasecmp(array[i].product_info.name, array[i+1].product_info.name) <= 0) {
                i++;
            }
            int end2 = i;
            i++;

            int p1 = start1, p2 = start2, k = start1;
            while (p1 <= end1 && p2 <= end2) {
                if (strcasecmp(array[p1].product_info.name, array[p2].product_info.name) <= 0) {
                    temp[k++] = array[p1++];
                } else {
                    temp[k++] = array[p2++];
                }
            }
            while (p1 <= end1) temp[k++] = array[p1++];
            while (p2 <= end2) temp[k++] = array[p2++];

            for (int j = start1; j <= end2; j++) {
                array[j] = temp[j];
            }
        }
    }
    free(temp);
}

void SortAndIndex(Country countriesUnordered[], int country_number, City_Index city_index[], Product_Index product_index[]) {
    
    for (int i = 0; i < country_number; i++) {
        int city_number = countriesUnordered[i].city_number;

        City sorted_cities[city_number];
        RSS_City(countriesUnordered[i].cities, city_number, sorted_cities);//C'de metoda array atınca kendi değerini değiştiriyomuş(Pass by Reference). Gemini javada da aynısı olduğunu iddia ediyo.Yersen.
        Merge_Runs_Cities(sorted_cities, city_number);

        // Ülkenin ilk şehrinin offset'ini belirle
        countriesUnordered[i].offset_CityIndex = allCity_number; //Country'nin offset_CityIndex'i

        for (int j = 0; j < city_number; j++) { //şehirleri City_Index'e kaydedicez
            int current_c_idx = allCity_number;
            
            city_index[current_c_idx].self_index = current_c_idx;
            city_index[current_c_idx].city = sorted_cities[j];
            
            if (j == city_number - 1) {city_index[current_c_idx].next_city_index = -1;} //son şehirse next_city_index -1 olur
            else {city_index[current_c_idx].next_city_index = current_c_idx + 1;}

            int product_number = sorted_cities[j].product_number;
           
            city_index[current_c_idx].product_index = allProduct_number; //Citynin product_index'i

            Product sorted_products[product_number];
            RSS_Product(sorted_cities[j].products, product_number, sorted_products);
            Merge_Runs_Products(sorted_products, product_number);

            for (int k = 0; k < product_number; k++) { //ürünleri product_indexe kaydetcez
                int current_p_idx = allProduct_number;
                    
                product_index[current_p_idx].self_index = current_p_idx;
                product_index[current_p_idx].product = sorted_products[k];
                    
                if (k == product_number - 1) {product_index[current_p_idx].product_offset = -1;} //son ürünse product_offset -1 olur
                else {product_index[current_p_idx].product_offset = current_p_idx + 1;}
                    
                product_index[current_p_idx].dat_offset = 0; //product_indexteki değil, productın kendisindeki dat_offseti kullanmaya karar verdim, ama bu dursun
                    
                allProduct_number++;   
            }
            allCity_number++; 
        }
    }
}

#pragma endregion

  

int main(int argc, char* argv){

  #pragma region JsonParse

  FILE *datFile = fopen("Binary.dat", "wb");

  struct json_object *parsed_json = json_object_from_file("Assignment -2.json");

  int country_number = json_object_array_length(parsed_json);
  Country *countriesUnordered = malloc(country_number * sizeof(Country)); //countriesi heap belleğinde oluşturuyoruz, öyle yapmayınca segmentation fault veriyor

  for (int i = 0; i < country_number; i++){ //country döngüsü

    Country *curCountry = &countriesUnordered[i]; //curCountry'i countriesUnordered'ın ilk yerine koyuyoruz
    curCountry->city_number = 0;
    struct json_object *country_obj = json_object_array_get_idx(parsed_json,i); //i inci countryi alır
    struct json_object *country, *country_code, *cities; //countrynin içindekiler

    json_object_object_get_ex(country_obj, "country", &country); 
    json_object_object_get_ex(country_obj, "country_code", &country_code);
    json_object_object_get_ex(country_obj,"cities", &cities);

    if(country != NULL) strcpy(curCountry->country,json_object_get_string(country)); //segmentation fault NULLu strcpy yapmaya çalışınca da çıkabiliyomuş
    if(country_code != NULL) strcpy(curCountry->country_code, json_object_get_string(country_code));

    int city_number = json_object_array_length(cities);
    for (int j = 0; j < city_number; j++){ //city döngüsü
      City *curCity = &curCountry->cities[j];
      curCity->product_number = 0;
      struct json_object *city_obj = json_object_array_get_idx(cities,j);
      struct json_object *city_name, *meta,*population,*region, *products;

      json_object_object_get_ex(city_obj,"city_name", &city_name);
      json_object_object_get_ex(city_obj, "meta", &meta);
      json_object_object_get_ex(meta,"population", &population);
      json_object_object_get_ex(meta,"region", &region);
      json_object_object_get_ex(city_obj,"products", &products);

      if(city_name != NULL) strcpy(curCity->city_name, json_object_get_string(city_name));
      if(region != NULL) strcpy(curCity->meta.region, json_object_get_string(region));
      curCity->meta.population = json_object_get_int64(population);

      //city bitti product yapcan şimdi. sonrasında countrye cityi, citye productı dahil etmeyi unutma
      int product_number = json_object_array_length(products);
      for (int k = 0; k < product_number; k++){
        Product *curProduct = &curCity->products[k];
        struct json_object *product_obj = json_object_array_get_idx(products,k);
        struct json_object *product_id, *product_info, *name, *brand, *category, *pricing, *price, *currency, *inventory, *stock, *warehouse, *isbn, *description, *extra;

        json_object_object_get_ex(product_obj,"product_id", &product_id);
        json_object_object_get_ex(product_obj,"product_info", &product_info);
        json_object_object_get_ex(product_info, "name", &name);
        json_object_object_get_ex(product_info, "brand", &brand);
        json_object_object_get_ex(product_info, "category", &category);
        json_object_object_get_ex(product_obj,"pricing", &pricing);
        json_object_object_get_ex(pricing, "price", &price);
        json_object_object_get_ex(pricing, "currency", &currency);
        json_object_object_get_ex(product_obj,"inventory", &inventory);
        json_object_object_get_ex(inventory, "stock", &stock);
        json_object_object_get_ex(inventory, "warehouse", &warehouse);
        json_object_object_get_ex(product_obj,"isbn", &isbn);
        json_object_object_get_ex(product_obj,"description", &description);
        json_object_object_get_ex(product_obj,"extra", &extra);

        if(product_id != NULL) strcpy(curProduct->product_id, json_object_get_string(product_id));
        if(name != NULL) strcpy(curProduct->product_info.name, json_object_get_string(name));
        if(brand != NULL) strcpy(curProduct->product_info.brand, json_object_get_string(brand));
        if(category != NULL) strcpy(curProduct->product_info.category, json_object_get_string(category));
        if(price != NULL) curProduct->pricing.price = json_object_get_int(price);
        if(currency != NULL) strcpy(curProduct->pricing.currency, json_object_get_string(currency));
        if(stock != NULL) curProduct->inventory.stock = json_object_get_int(stock);
        if(warehouse != NULL) strcpy(curProduct->inventory.warehouse, json_object_get_string(warehouse));
        if(isbn != NULL) strcpy(curProduct->isbn, json_object_get_string(isbn));
        if(description != NULL) strcpy(curProduct->description, json_object_get_string(description));
        if(extra != NULL) strcpy(curProduct->extra, json_object_get_string(extra));

        long bytePosition = ftell(datFile); //datFileda nerde kaldık
        curProduct->dat_offset = bytePosition;

        curCity->product_number++; 
        fwrite(curProduct, sizeof(Product), 1, datFile); //product datFile'a yazıldı
      }
       curCountry->city_number++;
    }
  } fclose(datFile); json_object_put(parsed_json);
  #pragma endregion
  
  #pragma region Sort

  City_Index city_index[100];
  Product_Index product_index[900];
  Country countries[10];
  
    for (int i = 0; i < country_number; i++) { //countriesUnordered countries'e kopyalanır
        countries[i] = countriesUnordered[i];
    }

    // 2. Ülkeleri Alfabetik Olarak Sırala (Basit Bubble Sort yeterlidir)
    for (int i = 0; i < country_number - 1; i++) {
        for (int j = 0; j < country_number - i - 1; j++) {
            if (strcasecmp(countries[j].country, countries[j+1].country) > 0) {
                Country temp = countries[j];
                countries[j] = countries[j+1];
                countries[j+1] = temp;
            }
        }
    }
  SortAndIndex(countries, country_number, city_index, product_index);

  #pragma endregion

    while(true){ //main menu döngüsü
      printf("1.Search by Country/City/Product\n2.Sort and Display Index Levels\n3.Insert a New Product\n4.Apply Replacement Selection Sort\n5.Exit\n");
      int menu;
      scanf("%d", &menu);

      
      
      if(menu == 1){ //search by country/city/product

        while(true){

        int searchInt = 0;
        while(true){ //hangisini aratmak istiyosa
         printf("\n1.Search Country\n2.Search City\n3.Search Product\n0.MAIN MENU\n");
         scanf("%d",&searchInt);
         if(searchInt != 1 && searchInt != 2 && searchInt != 3 && searchInt != 0){printf("Please enter a number between 0-3\n");}
         else{break;}  
        }
        char searchString[15];

        if(searchInt == 1){ //Country Search
          printf("Enter a country name: ");
          scanf(" %49[^\n]", searchString); //paşam boşluk okusun
          
          Country *country = BinarySearch_Country(countries,country_number, searchString); 
          if(country == NULL){printf("Country not found. \n");}
          else{
          printf("COUNTRY NAME: %s\nCOUNTRY CODE: %s\nCITIES: \n",country->country,country->country_code);
          for (int i = 0; i < country->city_number; i++){
            printf("%s\n",country->cities[i].city_name);
          }      
        }
        }

        else if(searchInt == 2){ //City Search //buraya see product details gibi bişey ekleyebilirim
          printf("Enter a city name: ");
          scanf("%s",searchString);

          FoundCity *foundcity = Search_City(countries, country_number, searchString, city_index);    
          if(strcmp(foundcity->city.city_name, "City not found.") == 0){printf("City not found");}
          else{
             printf("\nCity name: %s in %s\nPopulation: %d\nRegion: %s\nPRODUCTS:\n",foundcity->city.city_name,foundcity->country.country,foundcity->city.meta.population,foundcity->city.meta.region);
             for (int i = 0; i < foundcity->city.product_number; i++){
               printf(foundcity->city.products[i].product_info.name);
               printf("\n");
             }
          }
          free(foundcity); //bellek işleri beni yoruyo ya
        }

        else if(searchInt == 3){ //Product Search
          printf("Enter a product name: ");
          scanf(" %49[^\n]", searchString); //bu garip şeyin nedeni birden fazla kelime almak
         /* if(searchString[0] < 'A' || searchString[0] > 'Z'){ //user ilk harfi küçük girdiyse onu düzeltiyoz
            searchString[0] = searchString[0] - 32;
          }*/

          FoundProduct *foundproduct = Search_Product(countries, country_number,searchString,city_index,product_index); 
          if(strcmp(foundproduct->product.product_info.name, "Product not found.") == 0){printf("Product not found");}
          else{
            printf("\n%s,%s\n",foundproduct->city.city_name,foundproduct->country.country);
            printProductWithHeading(&(foundproduct->product));
          }
          free(foundproduct);
        }

        else{break;} //return to main menu
        getchar();
      }
      }
    
      else if(menu == 2){ //sort and display index level
        while(true){ //menu == 2nin döngüsü

          int scanInt1 = 0; 
        while(true){ //scanin döngüsü
          printf("\n1.Country Index Level\n2.City Index Level\n3.Product Index Level\n0.MAIN MENU\n");
          scanf("%d",&scanInt1);
          if(scanInt1 != 1 && scanInt1 != 2 && scanInt1 != 3 && scanInt1 != 0){printf("Please enter a number between 0-3\n"); continue;}
          else{break;}
        }

        if(scanInt1 == 0){break;}

        else if(scanInt1 == 1){ //Display sorted country
          printf("%-10s %20s\n","COUNTRY","Offset City Index");
          for (int i = 0; i < country_number; i++){
            printf("%-20s %-10d\n",countries[i].country,countries[i].offset_CityIndex);
          }  
        }
        else if(scanInt1 == 2){ //Display sorted city
          for (int i = 0; i < country_number; i++){
            printf("\nCountry: %s\n", countries[i].country);
            printf("%s %-15s %-9s %-13s\n","#", "CITY", "NEXT CITY", "PRODUCT INDEX");

            int offset = countries[i].offset_CityIndex;
            while(offset > -1){
              printf("%d %-15s %-9d %-13d\n", offset, city_index[offset].city.city_name, city_index[offset].next_city_index, city_index[offset].product_index);
              offset = city_index[offset].next_city_index;
            }
          }
        }
        else if(scanInt1 == 3){ //Display sorted product
          for (int i = 0; i < country_number; i++){
            
            int offset = countries[i].offset_CityIndex;
            while(offset > -1){ //cityi gösteren offset
              City city = city_index[offset].city;
              printf("\n%s, %s\n",city.city_name, countries[i].country); //şehir,ülke
              printf("%s %-25s %-14s %-12s\n","#", "PRODUCT NAME", "PRODUCT OFFSET", ".dat OFFSET");
              
              int p_offset = city_index[offset].product_index;
              while(p_offset > -1){ //productı gösteren offset
                printf("%d %-25s %-14d %-12ld\n", p_offset, product_index[p_offset].product.product_info.name, product_index[p_offset].product_offset,product_index[p_offset].product.dat_offset);
                p_offset = product_index[p_offset].product_offset;
              }
              offset = city_index[offset].next_city_index;
            }
            
          }

        }


        }
        
        

      }

      else if(menu == 3){ //insert a new product 

        char countryToAdd[50]; char cityToAdd[50]; Product newProduct;

        printf("Enter the Country of the new product: ");
        scanf("%s", countryToAdd);
        printf("Enter the City of the new product: ");
        scanf("%s", cityToAdd);

        int countryIndex= -1;
        for (int i = 0; i < country_number; i++) { //countryToAdd'in indexini buluyoruz
            if (strcasecmp(countries[i].country, countryToAdd) == 0) {
                countryIndex = i; break;
            }
        }

        if (countryIndex == -1) { //country bulamadıysak
            printf("\nCountry not found!\n\n");
            continue;
        }

        int city_offset = -1; //cityToAdd'in indexi
        int curCity_offset = countries[countryIndex].offset_CityIndex;
        while (curCity_offset != -1) { //cityToAdd'in city_indextteki yerini buluyoruz
            if (strcasecmp(city_index[curCity_offset].city.city_name, cityToAdd) == 0) {
                city_offset = curCity_offset; break;
            }
            curCity_offset = city_index[curCity_offset].next_city_index;
        }

        if (city_offset == -1) {printf("\nCity not found in %s\n\n", countries[countryIndex].country); continue;} //city bulunamadıysa

        printf("Enter new product name: ");
        scanf(" %[^\n]", newProduct.product_info.name); //bu da boşluklu alıyo 
        
        if(newProduct.product_info.name[0] >= 'a' && newProduct.product_info.name[0] <= 'z'){ //ismin ilk harfini büyültüyorum
            newProduct.product_info.name[0] -= 32;
        }

       char choice[1]; //product details seçimi
       while(true){
        printf("\nDo you want to define more product properties? (y/n) ");
        scanf("%s", choice);
        if(strcasecmp(choice, "y") != 0 && strcasecmp(choice, "n") != 0){
            printf("\nEnter 'y' for Yes, 'n' for No\n"); 
        }
        else{break;}
       }
       if(strcasecmp(choice, "y") == 0){
         printf("Enter Product ID: ");
         scanf("%s", newProduct.product_id);
         printf("Enter brand: ");
         scanf("%s", newProduct.product_info.brand);
         printf("Enter category: ");
         scanf("%s", newProduct.product_info.category); 
         printf("Enter price: ");
         scanf("%d", &newProduct.pricing.price); //integerda & koymayınca segfault veriyo
         printf("Enter currency: ");
         scanf("%s", newProduct.pricing.currency);
         printf("Enter stock: ");
         scanf("%d", &newProduct.inventory.stock);
         printf("Enter warehouse: ");
         scanf("%s", newProduct.inventory.warehouse);
         printf("Enter isbn: ");
         scanf("%s", newProduct.isbn);
         printf("Enter description: ");
         scanf("%s", newProduct.description);
         printf("Enter extra: ");
         scanf("%s", newProduct.extra);  
       }
       else if(strcasecmp(choice, "n") == 0){ //hepsi boş No ise
        strcpy(newProduct.product_id, "");
        strcpy(newProduct.product_info.brand, "");
        strcpy(newProduct.product_info.category , "");
        strcpy(newProduct.pricing.currency, "");
        strcpy(newProduct.inventory.warehouse, "");
        strcpy(newProduct.isbn, "");
        strcpy(newProduct.description, "");
        strcpy(newProduct.extra, "");
        newProduct.inventory.stock = 0;
        newProduct.pricing.price = 0;
       }

        FILE *datFile = fopen("Binary.dat", "ab"); //ab: append binary
        long new_byte_position = ftell(datFile);
        newProduct.dat_offset = new_byte_position;
        fwrite(&newProduct, sizeof(Product), 1, datFile);
        fclose(datFile);
        
        int newProductIndex = allProduct_number; //newProduct'ın indexi atanır
        
        product_index[newProductIndex].self_index = newProductIndex;
        product_index[newProductIndex].product = newProduct;
        product_index[newProductIndex].product_offset = -1; //ilk önce newProductı en sona koyuyoruz
        
        int curr_p_offset = city_index[city_offset].product_index; //ürünleri gezerken kullancaz
        int prev_p_offset = -1;

        while (curr_p_offset != -1) {
            if (strcasecmp(newProduct.product_info.name, product_index[curr_p_offset].product.product_info.name) < 0) { 
                break; //newProduct baktığımız üründen küçükse yani önceyse indexi bulduk, newProduct prev ile curr'ün arasına gelecek
            }
            prev_p_offset = curr_p_offset;
            curr_p_offset = product_index[curr_p_offset].product_offset;
        }

        if (prev_p_offset == -1) { //newProduct en başa eklenecekse city'nin product indexi olarak da ayarlamamız gerekiyo
            product_index[newProductIndex].product_offset = city_index[city_offset].product_index;
            city_index[city_offset].product_index = newProductIndex;
        } else { //newProduct araya bi yere eklenecekse
            product_index[newProductIndex].product_offset = curr_p_offset; //newProduct curr'ü gösterecej
            product_index[prev_p_offset].product_offset = newProductIndex; //prev newProductı gösterecek
        }

        city_index[city_offset].city.product_number++; //product_number artar
        allProduct_number++; 
        
        printf("\nProduct '%s' is inserted succesfully.\n", newProduct.product_info.name);

      }

      else if(menu == 4){ //apply replacement selection sort
            
            Visualized_RSS_Countries(countriesUnordered, country_number, countries);
            printf("Runs are built properly. Merge starts...\n");
            
            Visualized_Merge_Countries(countries, country_number); getchar();
      }

      else if(menu == 5){break;} //exit //burda yapmam gereken dah afazla şey var muhtemelen

      else{
        printf("Enter a number between 1-5");
        continue;
      }

    }


}