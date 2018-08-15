# OS_FINAL: Network Filesystem

Mikheil Zhghenti   mzhgh14@freeuni.edu.ge


პროექტი კომპილირდება *make*-ს საშუალებით. </br>
ქსელურ ფაილურ სისტემას აქვს ორი ძირითადი კომპონენტი, ესენია: კლიენტი და სერვერი. 

თითოეული მათგანის გაშვების სინოფსისის მაგალითები: </br>
**კლიენტი** *./net_raid_client client_config.txt* და </br>
**სერვერი** *./net_raid_server 127.0.0.1 10001 /home/misho/Desktop/storage1*

**სერვერის ზოგადი მიმოხილვა** </br>
სერვერი argv-დან შესაბამის პარამეტრებს იღებს და bind/listen-ით სტარტავს კონკრეტულ გადმოცემულ პორტზე "მომსახურებას". სერვერის ეფექტურ და უსაფრთხო ფუნქციონირებას(იმ გაგებით, რომ თუ კონკრეტულ request-ზე დაიქრაშა ჩვენი პროგრამა, სერვერი არ წყვეტს მუშაობას და აქტიური ახალი რექვესტის მისაღებად. ეს იმით, რომ თითოეულ მოთხოვნას ცალკე პროცესში ემსახურება.) სერვერის მხარეს შედარებით ნაკლები "საქმეა"(ლოგიკა მის მხარეს არაა დიდი რაოდენობით გატანილი), მას უფრო "კონკრეტული დავალებების" დამუშავება უწევს. მივმართავთ სერვერს, რომ შეასრულოს A სამუშაო: მიიღებს სერვერი A სამუშაობს, ასრულებს და შედეგს უკან გვიბრუნებს.

**კლიენტის ზოგადი მიმოხილვა**</br>
კლიენტის მთავარი მიზანი, არის მოცემული *task*-ის მართვა, მენეჯმენტი. უდიდესი ლოგიკაა სწორედ მის მხარესაა გატანილი. იგი იღებს გადაწყვეტილებას, თუ როგორ მოექცეს data-ს, როგორ შეინახოს და მაავე დროს performance-ზე ფიქრიც უწევს.

**პროექტის არქიტექტურა** </br>
ერორ ჰენდლინგი, ერრორების დაბრუნება. ფაილის გახნზე... terminal fprintef() LOG/debug

**უტილ კლასები**</br>

**ლოგირება / კლასების დეკომპოზიცაა**</br>
**გამოყენებული კლასები და მეთოდები** </br>

**დეტალურად კლიენტი**</br>
კლიენტის გამოძახებისას, მას პარამეტრად გადმოეცემა configuration-ის მისამართი, სადაც ჩამოწერილია ყელა სერვერის მისამართი, სტორიჯების სახელები, ლოგირების ფაილის სახელი, ქეშის ზომა, თაიმაუთი, დასამაუნთებელი დირექტორიები და სხვა. </br>

პროექტის სტრუქტურის მოწყობა ისე გადავწყვიტე, რომ თითოეულ Storage-ზე კონკრეტული 1 პროცესი მუშაობდეს. ეს უზრუნველყოფს იმას, რომ (ზემოთ უკვე ავღნიშნეთ), დამოუკიდებელი პროცესები საერთოდ "პრობლემას" უფრო ნაკლებად წარმოქმნიან. და თითოეული პროცესის მუშაობა საერთო ჯამში უფრო ეფექტური გამოვა. </br>

აღსანიშნავია, რომ ყოველ პროცესში "საზიარო" მონაცემბს წარმოადგენს: errorlog, cache_size, cache_replacment, timeout. ხოლო მათ გარდა, კონკრეტულ Storage-ს მონაცემები, მხოლოდ კონკრეტულ პროცესს აინტერესებს. სწორედ ამიტომ, თითოეული Storage-ს სრული მონაცემების წაკითხვის შემდეგ (diskname, mountpoint, raid, servers, hotswap) მშობელი პროცესი *fork()*-ს აკეთებს და შვილი პროცესი მხოლოდ კონკრეტულ 1 პროცესსზე იქნება პასუხისმგებელი.
```c
struct Client {
   char* error_log;
   int cache_size;
   char* cache_replacment;
   int timeout;
};
```
მაქვს Client-ის ტიპის სტრუქტურა და მასში ვინახავ ამ მონაცემებს. (იხ. *client.h*)

*fork()*-ი იძახება მაშინ როცა კონკრეტულ *Storage*-ზე სრული ინფორმაცია გვაქვს, ანუ ვიცით ყველა სერვერის მისამართი.
თითოეული სერვერის შესახებ მონაცემს ასეთ სტრუქტურაში ვინახავ:
```c
struct Server{
	char* ip;
	char* port;
};
```

ხოლო ყველა წაკითხულს ერთად კი მასივში ვინახავ(გლობალური ცვლადი *net_raid_client.c*-ში):
```c
struct Server servers[10];
```

უკვე მზად არის, რომ სერვერთან "ინიციალიზაცია" მოვახდინო კავშირის და descriptor-ები "მოვიპოვო". ამისთვის ვიყენებ socket()-ს:
```c
socket(AF_INET, SOCK_STREAM, 0);
```
და მის დაბრუნებულ *sfd*-ს ასევე გლობალურ მასივში ვინახავ, რომ შემდეგ ამის მეშვეობით კომუნიკაცია ვიქონიო სერვერთან.
```c
int servers_sfd [10];
```
connect()-ის გამოძახების შემდეგ ვნახულობ თუ "კავშირზეა" სერვერი. თუ წარმატებით განხორციელდა დაკავშირება **LOG**-ში იწერება შესაბამისი მონაცემ. აღსანიშნავია, რომ აქვე ხდება დაკავშირება **hotswap server**-თან.

სერვერებთან "შეხების" შემდეგ გადავდივართ **mount**-ზე. ვიძახებთ 
```c 
fuse_main(argc, new_argv, &all_methods, NULL); 
```
თუკი რაიმე პრობლემაა, დირექტორია ვერ იპოვნა, ან იპოვნა და უკვე დამაუნთებულია და სხვა პროცესი იყენებს მაშინ მეთოდი არანულოვანი მნიშვნელობით ბრუნდება და ესეც ილოგება შესაბამისი მესიჯით **LOG** ფაილში. და იდეაში, პროგრამა ამით ასრულებ მუშაობას, რადგან სამუშაოს ასე ვერ შეასრულებს.

ესაა ყველა ის **syscall**-ი, რომელიც გადატვირთულია სისტემაში:
```c
static struct fuse_operations all_methods = {
	.getattr	= my_getattr,
	.readdir	= my_readdir,
	.open		= my_open,
	.read		= my_read,
	.write      = my_write,
	.rename     = my_rename,
	.release    = my_release,
	.releasedir = my_releasedir,
	.rmdir      = my_rmdir,
	.mkdir      = my_mkdir,
	.unlink     = my_unlink,
	.create     = my_create,
	.utimens    = my_utimens,
	.opendir    = my_opendir,
};
```

*სერვერთან კომუნიკაციის პროტოკოლი* </br>
თითოეულ გადატვირთულ მეთოდში *printf()*-ით console-ში ვბეჭდავ კონკრეტული მეთოდის სახელს, პროცესის ID-ს და სხვა პარამეტრებს, რომლებიც გადმოეცემა.
მაგალითისთვის ასეთი ფორმტატით იქნება *getattr*-ის შემთხვევაში:
```c
printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "getattr",path);
```
(ეს მიმარტივებდა DEBUG-ის პროცესს და ლოგირებაშიც მეხმარებოდა)

თითოეული syscall-ის გამოძახებისას კლიენტი ადგენს Request-ს, სადაც წერია syscall-ის სახელი და path.
ამას ვაკეთებ შემდეგ ნაირიად, მაგალითისთვის:
```c
	char request [strlen("getattr")+strlen(path)+2];
	sprintf(request, "%s %s", "getattr", path); // sets values

```
და ამ request-ს ვგზავნი პირველ ეტაპზე.</br>

Server-ზე არის ამ მონაცემის *parser*-ი, რომელიც ადგენს თუ რომელი მეთოდი გამოვიძახოთ და რა პარამეტრით. (path-ის გადაცემა request-ში იმიტომ გადავწყვიტე, რომ ყველა იყენებს, ხოლო სხვა კონკრეტული პარამეტრები კი კომპლექსურია და ზოგი იყენებს, ზოგი - არა, და ამიტომ მათ ეტაპობრივად საჭიროემისამებრ ვგზავნი). </br>

Server-ზე გაწერილია "დიდი IF/ELSE IF/ELSE" სადაც კონკრეტულ syscall-ს ასრულებს და შედეგს უკან კლიენტს უბრუნებს. </br>








