<?php
class Sample3Fuse extends Fuse {
  public function __construct() {
    printf("phpfuse: %s called\n",__FUNCTION__);

  }
  public function __destruct() {
    printf("phpfuse: %s called\n",__FUNCTION__);
  } 
  public function getattr() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function readlink() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function getdir() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function mknod() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function mkdir() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function unlink() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function rmdir() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function symlink() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function rename() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function link() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function chmod() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function chown() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function truncate() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function utime() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function open() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function read() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function write() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function statfs() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function flush() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function release() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function fsync() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function setxattr() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function getxattr() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function listxattr() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
  public function removexattr() {
    printf("phpfuse: %s called\n",__FUNCTION__);
    return -FUSE_ENOSYS;

  }
}

echo "prog begin\n";
$fuse = new Sample3Fuse();
echo "mounting\n";
$fuse->mount("/tmp/phpfusemount", array("debug"));
echo "done\n";
?>
