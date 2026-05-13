#include<stdio.h>
#include<json-c/json.h>
#include <stdbool.h>
#include <string.h>

#define HEAP_MAX 4


#pragma region Structs
typedef struct _Product_Info{ //product object
    char name[64];
    char brand[64];
    char category[64];
  }Product_Info;

  typedef struct _Pricing{ //product object
    int price;
    char currency[16];
  }Pricing;
  
  typedef struct _Inventory{ //product object
    int stock;
    char warehouse[16];
  }Inventory;

  typedef struct _Product{
    char product_id[64];
    Product_Info product_info;
    Pricing pricing;
    Inventory inventory;
    char isbn[32];
    char description[128];
    char extra[64];
  }Product;

  typedef struct _Meta{ //city object
      int population;
      char region[32];
  }Meta;

  typedef struct _City{
    char city_name[16];
    Meta meta;
    Product products[32];
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

  typedef struct {
    int self_index;
    Product product;
    int product_offset;
    long dat_offset;
  } Product_Index;
  #pragma endregion

  #pragma region Search Operations //searchlerin bulunamadığı ihtimali yapmayı unutma Countryde

  typedef struct{ //SearchCity için
    City city;
    Country country;
  }FoundCity;

  typedef struct{ //SearchCountry için
    Country country;
    City city;
    Product product;
  }FoundProduct;


   Country BinarySearch_Country(Country *countries[], int country_number, char countryName[]){
    int middle = country_number/2; int end = country_number; int start = 0;
    while(true){
      Country compareCountry = *countries[middle];
      if(strcmp(countryName, compareCountry.country) > 0){ //arattığımız country daha büyükse sağa geçer
        start = middle; middle = (end-start)/2;
      }
      else if(strcmp(countryName, compareCountry.country) < 0){ //arattığımız country daha küçükse sola geçer
        end = middle; middle = (end-start)/2;
      }
      else{ //countryi bulduysak
        return compareCountry;
      }
    }
  }

  FoundCity Search_City(Country *countries[], int country_number, char cityName[], City_Index city_index[]){
    FoundCity foundcity;
    for (int i = 0; i < country_number; i++) {
        int offset = countries[i]->offset_CityIndex;
        
        while (offset != -1) {
            if (strcmp(cityName, city_index[offset].city.city_name) == 0) {
              foundcity.city = city_index[offset].city; //bulunan şehri kaydet
              foundcity.country = *countries[i]; //bulunan şehrin ülkesini kaydet
              return foundcity;
            }
            offset = city_index[offset].next_city_index;
        }
    }
    
    strcpy(foundcity.city.city_name, "City not found."); //city bulunmadıysa
    return foundcity;
  }

  FoundProduct Search_Product(Country *countries[], int country_number, char productName[], City_Index city_index[], Product_Index product_index[]){
    FoundProduct foundproduct;
    for (int i = 0; i < country_number; i++) {
        int offset = countries[i]->offset_CityIndex;
        
        while (offset != -1) {
            int p_offset = city_index[offset].product_index;
            
            while (p_offset != -1) {
                if (strcmp(productName, product_index[p_offset].product.product_info.name) == 0) {
                  foundproduct.product = product_index[p_offset].product; //bulunan productı kaydet
                  foundproduct.city = city_index[offset].city; //bulunan productın citysini kaydet
                  foundproduct.country = *countries[i]; //bulunan productın ülkesini kaydet
                    return foundproduct;
                }
                p_offset = product_index[p_offset].product_offset; //diğer ürüne geçiyo
            }
            offset = city_index[offset].next_city_index; //diğer şehre geçiyo
        }
    }
    
    
    strcpy(foundproduct.product.product_info.name, "Product not found.");
    return foundproduct;
  }


  #pragma endregion

#pragma region Heap

  typedef struct{
    Country data;
  } Node;

  void heapify(Node *heap[]){ 
    int leftChild = 0, rightChild = 0; bool isRightChild = true;
    for (int i = 0; i < HEAP_MAX; i++){
      if(i == 0){leftChild = 1; rightChild = 2;}
      else{leftChild = 2*i; rightChild = 2*i+1;}

      if(isRightChild){
        isRightChild = reheap(*heap, i, rightChild);
      }
      else{
        isRightChild = !reheap(*heap,i,leftChild);
      }
    }
  }

  bool reheap(Node *heap[], int indexParent, int indexChild){ //bu metot sadece verilen indexleri karşılaştırıp gerekirse yer değiştirir
    bool flag = false;
    if(strcmp((*heap[indexParent]).data.country, (*heap[indexChild]).data.country) > 0){
      Node temp = *heap[indexParent];
      *heap[indexParent] = *heap[indexChild];
      *heap[indexChild] = temp;
      flag = true;
    }
    return flag;
  }

  void insertToHeap(Node *heap[], Node insertNode){ //heapin 0. nodeuna insert yapar
    *heap[0] = insertNode;
    heapify(*heap);
  }

 #pragma endregion

 void printProductWithHeading(Product p){
  printf("%-16s %-16s %-10s %-16s %-16s %-16s %-20s %-10s %s\n","NAME", "BRAND", "PRICING", "CATEGORY", "ID", "INVENTORY", "ISBN", "DESCRIPTION", "EXTRA");
  printf("%-16s %-16s %-5s %-5s %-16s %-16s %-6s %-6s %-20s %-10s %s",
    p.product_info.name,p.product_info.brand,p.pricing.price,p.pricing.currency,p.product_info.category,p.product_id,p.inventory.warehouse,p.inventory.stock,p.isbn,p.description,p.extra);
 }

 void printProduct(Product p){
  printf("%-16s %-16s %-5s %-5s %-16s %-16s %-6s %-6s %-20s %-10s %s",
    p.product_info.name,p.product_info.brand,p.pricing.price,p.pricing.currency,p.product_info.category,p.product_id,p.inventory.warehouse,p.inventory.stock,p.isbn,p.description,p.extra);
 }

 void printAllProducts(City city){
  printf("%-16s %-16s %-10s %-16s %-16s %-16s %-20s %-10s %s\n","NAME", "BRAND", "PRICING", "CATEGORY", "ID", "INVENTORY", "ISBN", "DESCRIPTION", "EXTRA");
  for (int i = 0; i < city.product_number; i++){
    printProduct(city.products[i]);
  }
 }

   int ReplacementSelectionSort(Country countries[], char sortWhat [], int country_number){ //burayı yarım bırakıyorum

    Node *heap[HEAP_MAX]; Node *list[HEAP_MAX]; //heap ve list
    Node inputNode, outputNode; //country sort için input ve output
    Country *countriesOrdered[country_number];
    

    if(strcmp(sortWhat, "country") == 0){ //countrynin sortu her an silebilirim bok gibi yazdım
      for (int i = 0; i < country_number; i++){
      Node insertNode;
      if(i <= HEAP_MAX){ //önce heapin içi doldurulur
        insertNode.data = countries[i];
        insertToHeap(*heap,insertNode);
      }
      else{ //heap doluysa
        inputNode.data = countries[i];
      if(strcmp(inputNode.data.country,outputNode.data.country) > 0){ //input > output
        outputNode = *heap[0];
        insertToHeap(*heap,insertNode);
        for (int j = 0; j < country_number; j++){ //countryOrdereda ekleme yapılır
          if(countriesOrdered[j] != NULL){
            countriesOrdered[j] = outputNode.data.country;
          }
        }
      }
      else{ //input < output
        insertToHeap(*heap, inputNode);
      }
      } 
    }
    int try = 0; int heapIndex = 0;
    while(countriesOrdered[try] != NULL){ //heapte son kalanlar countriesOrdereda atılır
      for (int i = 0; i < HEAP_MAX; i++){
        *countriesOrdered[try] = (*heap[i]).data;
      }
    }
  }




  }


int main(int argc, char* argv){

  #pragma region JsonParse

  FILE *datFile = fopen("Binary.dat", "wb");

  struct json_object *parsed_json = json_object_from_file("Assignment -2.json");
  int country_number = json_object_array_length(parsed_json);
  Country countriesUnordered [country_number];

  for (int i = 0; i < country_number; i++){ //country döngüsü
    Country curCountry;
    curCountry.city_number = 0;
    struct json_object *country_obj = json_object_array_get_idx(parsed_json,i); //i inci countryi alır
    struct json_object *country, *country_code, *cities; //countrynin içindekiler

    json_object_object_get_ex(country_obj, "country", &country); 
    json_object_object_get_ex(country_obj, "country_code", &country_code);
    json_object_object_get_ex(country_obj,"cities", &cities);

    strcpy(curCountry.country,json_object_get_string(country));
    strcpy(curCountry.country_code, json_object_get_string(country_code));

    int city_number = json_object_array_length(cities);
    for (int j = 0; j < city_number; j++){ //city döngüsü
      City curCity;
      curCity.product_number = 0;
      struct json_object *city_obj = json_object_array_get_idx(cities,j);
      struct json_object *city_name, *meta,*population,*region, *products;

      json_object_object_get_ex(city_obj,"city_name", &city_name);
      json_object_object_get_ex(city_obj, "meta", &meta);
      json_object_object_get_ex(meta,"population", &population);
      json_object_object_get_ex(meta,"region", &region);
      json_object_object_get_ex(city_obj,"products", &products);

      strcpy(curCity.city_name, json_object_get_string(city_name));
      strcpy(curCity.meta.region, json_object_get_string(region));
      curCity.meta.population = json_object_get_int64(population);

      //city bitti product yapcan şimdi. sonrasında countrye cityi, citye productı dahil etmeyi unutma
      int product_number = json_object_array_length(products);
      for (int k = 0; k < product_number; k++){
        Product curProduct;
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

        strcpy(curProduct.product_id, json_object_get_string(product_id));
        strcpy(curProduct.product_info.name, json_object_get_string(name));
        strcpy(curProduct.product_info.brand, json_object_get_string(brand));
        strcpy(curProduct.product_info.category, json_object_get_string(category));
        curProduct.pricing.price = json_object_get_int(price);
        strcpy(curProduct.pricing.currency, json_object_get_string(currency));
        curProduct.inventory.stock = json_object_get_int(stock);
        strcpy(curProduct.inventory.warehouse, json_object_get_string(warehouse));
        strcpy(curProduct.isbn, json_object_get_string(isbn));
        strcpy(curProduct.description, json_object_get_string(description));
        strcpy(curProduct.extra, json_object_get_string(extra));

        curCity.products[k] = curProduct; curCity.product_number++; //curProduct curCitye eklendi
        fwrite(&curProduct, sizeof(curProduct), 1, datFile); //product datFile'a yazıldı
      }
      curCountry.cities[j] = curCity; curCountry.city_number++;
    }
    countriesUnordered[i] = curCountry;
  } fclose(datFile); json_object_put(parsed_json);
  #pragma endregion
  


  #pragma region Sort

  City_Index city_index[100];
  Product_Index product_index[100];

  #pragma endregion









    while(true){ //main menu döngüsü
      printf("1.Search by Country/City/Product\n2.Sort and Display Index Levels\n3.Insert a New Product\n4.Apply Replacement Selection Sort\n5.Exit\n");
      int menu;
      scanf("%d", &menu);
      
      if(menu == 1){ //search by country/city/product

        while(true){

        int searchInt = 0;
        while(true){ //hangisini aratmak istiyosa
         printf("1.Search Country\n2.Search City\n3.Search Product\n0.MAIN MENU");
         scanf("%d",&searchInt);
         if(searchInt != 1 && searchInt != 2 && searchInt != 3 && searchInt != 0){printf("Please enter a number between 0-3"); continue;}
         else{break;}  
        }
        char *searchString[15];

        if(searchInt == 1){ //Country Search
          printf("Enter a country name");
          scanf("%s",searchString);
          if(searchString[0] < 'A' || searchString[0] > 'Z'){ //user ilk harfi küçük girdiyse onu düzeltiyoz
            searchString[0] = searchString[0] - 32;
          }

          Country country = BinarySearch_Country(countriesUnordered,country_number, searchString); 
          printf("Country name: %s\nCountry Code: %s\nCities: ",country.country,country.country_code);
          for (int i = 0; i < country.city_number; i++){
            printf(country.cities[i].city_name);
            printf(" Products:");
            for (int j = 0; j < country.cities[i].product_number; j++){
              printf((country.cities[i]).products[j].product_info.name);
            }
          }          
        }

        else if(searchInt == 2){ //City Search
          printf("Enter a city name");
          scanf("%s",searchString);
          if(searchString[0] < 'A' || searchString[0] > 'Z'){ //user ilk harfi küçük girdiyse onu düzeltiyoz
            searchString[0] = searchString[0] - 32;
          }

          FoundCity foundcity = Search_City(countriesUnordered, country_number, searchString, city_index); //buraya da yine countriesUnordered, sonra düzelt      
          if(strcmp(foundcity.city.city_name, "City not found.") == 0){printf("City not found");}
          else{
             printf("City name: %s in %s\n Products:",foundcity.city.city_name,foundcity.country.country);
             for (int i = 0; i < foundcity.city.product_number; i++){
               printf(foundcity.city.products[i].product_info.name);
             }
          }
        }

        else if(searchInt == 3){ //Product Search
          printf("Enter a product name");
          scanf("%s",searchString);
          if(searchString[0] < 'A' || searchString[0] > 'Z'){ //user ilk harfi küçük girdiyse onu düzeltiyoz
            searchString[0] = searchString[0] - 32;
          }

          FoundProduct foundproduct = Search_Product(countriesUnordered, country_number,searchString,city_index,product_index);
          if(strcmp(foundproduct.product.product_info.name, "Product not found.") == 0){printf("Product not found");}
          else{
            printProductWithHeading(foundproduct.product);
            printf("\n %s,%s",foundproduct.city.city_name,foundproduct.country.country);
          }

          

        }

        else{break;} //return to main menu
        getchar();
      }
      

        


      }

      else if(menu == 2){ //sort and display index level

      }

      else if(menu == 3){ //insert a new product

      }

      else if(menu == 4){ //apply replacement selection sort

      }

      else if(menu == 5){break;} //exit

      else{
        printf("Enter a number between 1-5");
        continue;
      }

    }


}