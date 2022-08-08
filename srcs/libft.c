#include "../includes/famine.h"

void	ft_bzero(void *s, size_t n)
{
	unsigned int	i;
	char			*tmp;

	i = 0;
	tmp = (char *)s;
	while (i < n)
	{
		tmp[i] = '\0';
		i++;
	}
}

int	ft_strcmp(const char *s1, const char *s2)
{
	int				i;

	i = 0;
	while (s1[i] || s2[i])
	{
		if (s1[i] != s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
}

char *ft_strcpy(char *dest, const char *src)
{
	int i;

	i = 0;
	while (src[i])
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

char *ft_strcat(char *dest, const char *src)
{
	int i;
	int j;

	i = 0;
	j = 0;
	while (dest[i])
		i++;
	while (src[j])
	{
		dest[i] = src[j];
		i++;
		j++;
	}
	dest[i] = '\0';
	return (dest);
}