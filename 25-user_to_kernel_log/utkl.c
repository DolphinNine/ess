/*Модуль ядра*/
/*
 * Помимо загрузки и выгрузки по команде (insmod utkl.ko | rmmod utkl),
 * модуль способен реагировать не действия с создаваемым им файлом-устройства,
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

/*----------------------------------------------------------------------------*/
/*Инициализация*/
static int __init utkl_init(void)
{
  pr_info("UTKL: loaded\n");

  /*Регистрация мажорного номера*/
  dev_major = register_chrdev(0, DEV_NAME, &fops);
  if (dev_major < 0)
  {
    pr_info("UTKL: unable to register dev\n");
    return -1;
  }
  pr_info("UTKL: dev registered with %d major number\n", dev_major);

  /*Создание числа из мажорного и минорного чисел. Они используются далее*/
  dev_n = MKDEV(dev_major, 0);

  /*Структура устройства, используемая в создании*/
  dev_class = class_create(THIS_MODULE, DEV_NAME);
  /*Создание самого устройства в системе*/
  dev_struct = device_create(dev_class, NULL, dev_n, NULL, DEV_NAME);

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
  static const char *hello_msg = "Kernel module says Hi!\n\r";
  size_t to_len;

  to_len = strlen(hello_msg);

  /*Используется simple_read... вместо copy_to_user. Таким образом фукнция
    сама контролирует число остаточных байтов*/
  /*Первые 3 агрумента (buf, len, ppos) получаются функцией при обращении
    из пользовательского пространства*/
  return (simple_read_from_buffer(buf, len, ppos, hello_msg, to_len));
}

/*----------------------------------------------------------------------------*/
/*Запись*/
static ssize_t utkl_write(struct file *file, const char __user *buf, size_t len,
  loff_t *ppos)
{
  static char from_user_msg[100];
  static char to_log_msg[200];
  static size_t to_len;

  memset(from_user_msg, 0, 0);
  memset(to_log_msg, 0, 0);

  /*Сообщение пользователя копируется в локальную переменную. Аналогично
    функции чтения, часть аргументов здесь - данные полученные из
    пользовательского пространства*/
  copy_from_user(&from_user_msg, buf, len);
  /*Формирование строки для записи. Сбор всего вывода в одну строку позволяет
    подсчитать потом общую длинну записанных данных*/
  sprintf(to_log_msg, "UTKL [USER]: %s\n", from_user_msg);
  to_len = strlen(to_log_msg);

  /*Вывод в логи ядра*/
  pr_info("%s", to_log_msg);
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
