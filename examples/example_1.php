<?php
class Sample1Fuse extends Fuse {
}

$fuse = new Sample1Fuse();
$fuse->mount("/tmp/phpfusemount", "allow_other");
?>
