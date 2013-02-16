<?php
class Sample3Fuse extends Fuse {
  function __call($name,$args) {
    echo "Function $name was called. Arguments:\n";
    var_dump($args);
    return FUSE_ENOSYS;
  }
}

$fuse = new Sample3Fuse();
$fuse->mount("/tmp/phpfusemount", "debug");
?>
