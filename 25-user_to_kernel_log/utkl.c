/*Модуль ядра*/
/*
 * Помимо загрузки и выгрузки по команде (insmod utkl.ko | rmmod utkl),
 * модуль способен реагировать на действия с создаваемым им файлом-устройства,
 * записанным в DEV_NAME(по-умолчанию "utkl_dev"):
 *
 * -Чтение устройства (например, cat /dev/utkl_dev) приведёт к передаче строки
 * hello_msg (по-умолчанию "Kernel module says Hi!") (пример с cat приведёт к
 * выводу строки в терминал);
 * -Запись в файл устройства (например, echo "hi" > /dev/utkl_dev) приведёт к
 * записи полученной строки в лог-файл ядра.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h> /*Определения макро*/
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/rwlock.h>

#define DEV_NAME "utkl_dev"

/*Прототипы*/
static int utkl_init(void);
static void utkl_exit(void);
static ssize_t utkl_read(struct file*, char __user*, size_t, loff_t*);
static ssize_t utkl_write(struct file*, const char __user*, size_t, loff_t*);

/*Переопределение функций*/
static const struct file_operations fops = {
  .owner = THIS_MODULE,
  .read = utkl_read,
  .write = utkl_write,
};

/*Хранение назначенного системой числа устройства*/
static dev_t dev_n;
static int dev_major;

/*Структуры для файла устройства. Создание идёт в utkl_init*/
static struct class *dev_class;
static struct device *dev_struct;

/*Замок, блокирующий чтение/запись устройства*/
rwlock_t dev_lock;

/*----------------------------------------------------------------------------*/
/*Инициализация*/
static int __init utkl_init(void)
{
  pr_info("UTKL: loaded\n");

  /*Регистрация мажорного номера*/
  dev_major = register_chrdev(0, DEV_NAME, &fops);
  if (dev_major <= 0)
  {
    /* Если произойдёт ошибка - будет возвращенно не положительное число.
    * В этом случае, все свершённые ранее изменения будут отменены и будет
    * возвращена ошибка.
    * Не текущем этапе ничего отменять не требуется.
    */
    pr_info("UTKL: unable to register dev\n");
    return -1;
  }
  pr_info("UTKL: dev registered with %d major number\n", dev_major);

  /*Создание числа из мажорного и минорного чисел. Они используются далее*/
  dev_n = MKDEV(dev_major, 0);

  /*Структура устройства, используемая в создании*/
  dev_class = class_create(THIS_MODULE, DEV_NAME);
  if(dev_class <= 0)
  {
    /* Ошибка - отменить успешно пройденную ранее регистрацию мажорного числа*/
    unregister_chrdev(dev_major, DEV_NAME);
    unregister_chrdev_region(dev_n, 1);
    pr_info("UTKL: unable to create dev class\n");
    return -1;
  }
  /*Создание самого устройства в системе*/
  dev_struct = device_create(dev_class, NULL, dev_n, NULL, DEV_NAME);
  if(dev_struct <= 0)
  {
    /* Ошибка - отменить регистрацию и уничтожить класс.*/
    class_destroy(dev_class);
    unregister_chrdev(dev_major, DEV_NAME);
    unregister_chrdev_region(dev_n, 1);
    pr_info("UTKL: unable to create dev\n");
    return -1;
  }

  /*Инициализация замка*/
  rwlock_init(&dev_lock);

  return 0;
}

/*----------------------------------------------------------------------------*/
/*Выгрузка*/
/*При выгрузке происходит удаление созданого файла-устройства*/
static void __exit utkl_exit(void)
{
  device_destroy(dev_class, dev_n);
  class_destroy(dev_class);
  unregister_chrdev(dev_major, DEV_NAME);
  unregister_chrdev_region(dev_n, 1);

  pr_info("UTKL: unloaded\n");
}

/*----------------------------------------------------------------------------*/
/*Чтение*/
static ssize_t utkl_read(struct file *file, char __user *buf, size_t len,
                        loff_t *ppos)
{
  /*Константа-строка, выводимая при обращении*/
  const char *hello_msg = "Kernel module says Hi!\n\0";
  size_t to_len, ret_len;

  to_len = strlen(hello_msg);

  /*Блокировка замка. Не так критичено на чтении, как на записи.*/
  read_lock(&dev_lock);
  /*Используется simple_read... вместо copy_to_user. Таким образом фукнция
  сама контролирует число остаточных байтов*/
  /*Первые 3 агрумента (buf, len, ppos) получаются функцией при обращении
  из пользовательского пространства*/
  ret_len = simple_read_from_buffer(buf, len, ppos, hello_msg, to_len);
  read_unlock(&dev_lock);

  return ret_len;
}

/*----------------------------------------------------------------------------*/
/*Запись*/
static ssize_t utkl_write(struct file *file, const char __user *buf, size_t len,
  loff_t *ppos)
{
  char from_user_msg[100];
  char to_log_msg[200];
  size_t to_len;

  /* Проверка на то, что полученное число не больше того, что может вместить
   * локальный буфер. Иначе - функция вернёт ошибку и окончит свою работу*/
  if(len > sizeof(from_user_msg)-1)
  {
    return -EOVERFLOW;
  }

  memset(from_user_msg, 0, sizeof(from_user_msg));
  memset(to_log_msg, 0, sizeof(to_log_msg));

  /* Блокировка замка. В этой функции занимает почти весь процесс, начиная от
   * копирования сообщения из пользовательского пространства, и заканчивая
   * печатью итогового сообщения в лог-файл.*/
  write_lock(&dev_lock);
  /*Сообщение пользователя копируется в локальную переменную. Аналогично
  функции чтения, часть аргументов здесь - данные полученные из
  пользовательского пространства*/
  /*copy_from_user(&from_user_msg, buf, len);*/
  simple_write_to_buffer(&from_user_msg, sizeof(from_user_msg), ppos, buf, len);
  /*Формирование строки для записи. Сбор всего вывода в одну строку позволяет
    подсчитать потом общую длинну записанных данных*/
  sprintf(to_log_msg, "UTKL [USER]: %s\0", from_user_msg);
  to_len = strlen(to_log_msg);

  /*Вывод в логи ядра*/
  pr_info("%s", to_log_msg);
  write_unlock(&dev_lock);

  /*Возврат записанных данных. Если возвращаемое число будет неверным - функция
  будет вызвана вновь, что породит бесконечный цикл записи строки в лог-файл*/
  return to_len;
}

/*Определение функций загрузки и выгрузки модуля*/
module_init(utkl_init);
module_exit(utkl_exit);

/*Информация о модуле*/
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Prints input of utkl dev file to kernel log");
