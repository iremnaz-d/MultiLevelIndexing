#include<stdio.h>
#include<json-c/json.h>
#include <stdbool.h>

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
  }City;

  typedef struct _Country{
    char country[16];
    char country_code[4];
    City cities[32];
  }Country;
  #pragma endregion


int main(int argc, char* argv){


  FILE *jsonFile = fopen("Assignment-2.json", "r");
  FILE *datFile = fopen("Binary.dat", "wb");

  struct json_object *parsed_json = json_object_from_file(jsonFile);
  int country_number = json_object_array_length(parsed_json);

  for (int i = 0; i < country_number; i++){ //country döngüsü
    Country curCountry;
    struct json_object *country_obj = json_object_array_get_idx(parsed_json,i); //i inci countryi alır
    struct json_object *country, *country_code, *cities; //countrynin içindekiler

    json_object_object_get_ex(country_obj, "country", country); 
    json_object_object_get_ex(country_obj, "country_code", country_code);
    json_object_object_get_ex(country_obj,"cities",cities);

    strcpy(curCountry.country,json_object_get_string(country));
    strcpy(curCountry.country_code, json_object_get_string(country_code));

    int city_number = json_object_array_length(cities);
    for (int j = 0; j < city_number; j++){ //city döngüsü
      City curCity;
      struct json_object *city_obj = json_object_array_get_idx(cities,j);
      struct json_object *city_name, *meta,*population,*region, *products;

      json_object_object_get_ex(city_obj,"city_name", city_name);
      json_object_object_get_ex(city_obj, "meta", meta);
      json_object_object_get_ex(meta,"population", population);
      json_object_object_get_ex(meta,"region",region);

      strcpy(curCity.city_name, json_object_get_string(city_name));
      strcpy(curCity.meta.region, json_object_get_string(region));
      curCity.meta.population = json_object_get_int64(population);

      //city bitti product yapcan şimdi. sonrasında countrye cityi, citye productı dahil etmeyi unutma

    }
  }
  










    while(true){
      printf("1.Search by Country/City/Product\n2.Sort and Display Index Levels\n3.Insert a New Product\n4.Apply Replacement Selection Sort\n5.Exit\n");
      int menu;
      scanf("%d", &menu);
      
      if(menu == 1){ //search by country/city/product

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