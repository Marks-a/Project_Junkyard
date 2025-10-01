provider "aws" {
  region = "us-east-1"
}

# S3 bucket (file storage)
resource "aws_s3_bucket" "app_bucket" {
  bucket = "my-customer-app-bucket-12345"
# acl    = "private"
}

# Security group to allow HTTP
resource "aws_security_group" "app_sg" {
  name        = "app-sg"
  description = "Allow HTTP traffic"

  ingress {
    from_port   = 80
    to_port     = 80
    protocol    = "tcp"
    cidr_blocks = ["0.0.0.0/0"]
  }

  egress {
    from_port   = 0
    to_port     = 0
    protocol    = "-1"
    cidr_blocks = ["0.0.0.0/0"]
  }
}

# EC2 (t2)
resource "aws_instance" "app_server" {
  ami           = "ami-08c40ec9ead489470" # Amazon Linux 2 in us-east-1
  instance_type = "t2.micro"
  security_groups = [aws_security_group.app_sg.name]

  # User data script: installs git, clones app repo, runs Flask
  user_data = <<-EOF
              #!/bin/bash
              yum update -y
              amazon-linux-extras install python3.8 git -y
              pip3 install flask boto3
              cd /home/ec2-user
              git clone https://github.com/YOUR_GITHUB_USERNAME/customer-app.git
              cd customer-app/app
              pip3 install -r req.txt
              python3 app.py &
              EOF

  tags = {
    Name = "CustomerAppServer"
  }
}

output "ec2_public_ip" {
  value = aws_instance.app_server.public_ip
}

output "s3_bucket" {
  value = aws_s3_bucket.app_bucket.bucket
}
